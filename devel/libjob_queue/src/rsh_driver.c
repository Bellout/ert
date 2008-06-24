#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <basic_queue_driver.h>
#include <rsh_driver.h>
#include <util.h>
#include <pthread.h>
#include <void_arg.h>
#include <errno.h>


struct rsh_job_struct {
  int 	       __basic_id;
  int  	       __rsh_id;
  int          node_index;
  bool         active;       /* Means that it allocated - not really in use */ 
  bool         running;      /* Means  it is currently running - should probably only use this? */
  pthread_t    run_thread;
  const char * host_name;    /* Currently not set */
  char       * run_path;
};



typedef struct {
  char 	     	  * host_name;
  int  	     	    max_running;
  int  	     	    running;
  pthread_mutex_t   host_mutex;
} rsh_host_type;



struct rsh_driver_struct {
  BASIC_QUEUE_DRIVER_FIELDS
  int                 __rsh_id;
  pthread_mutex_t     submit_lock;
  pthread_attr_t      thread_attr;
  char              * rsh_command;
  int                 num_hosts;
  rsh_host_type     **host_list;
};



/******************************************************************/


/**
   If the host is for some reason not available, NULL should be returned.
*/




static rsh_host_type * rsh_host_alloc(const char * host_name , int max_running) {
  rsh_host_type * host = util_malloc(sizeof * host , __func__);
  
  host->host_name   = util_alloc_string_copy(host_name);
  host->max_running = max_running;
  host->running     = 0;
  pthread_mutex_init( &host->host_mutex , NULL );

  return host;
}


/*
  static void rsh_host_reset(rsh_host_type * rsh_host) {
  rsh_host->running = 0;
}
*/

static void rsh_host_free(rsh_host_type * rsh_host) {
  free(rsh_host->host_name);
  free(rsh_host);
}


static bool rsh_host_available(rsh_host_type * rsh_host) {
  bool available;

  pthread_mutex_lock( &rsh_host->host_mutex );
  if ((rsh_host->max_running - rsh_host->running) > 0) {
    available = true;
    rsh_host->running++;
  } else
    available = false;
  pthread_mutex_unlock( &rsh_host->host_mutex );

  return available;
}





static void rsh_host_submit_job(rsh_host_type * rsh_host , rsh_job_type * job, const char * rsh_cmd , const char * submit_cmd , const char * run_path) {
  /* 
     Observe that this job has already been added to the running jobs
     in the rsh_host_available function.
  */

  job->running = true;
  printf("Calling: %s %s %s %s \n" , rsh_cmd , rsh_host->host_name , submit_cmd, run_path);
  util_vfork_exec(rsh_cmd , 3 , (const char *[3]) {rsh_host->host_name , submit_cmd , run_path} , true , NULL , NULL , NULL , NULL);
  job->running = false;

  pthread_mutex_lock( &rsh_host->host_mutex );
  rsh_host->running--;
  pthread_mutex_unlock( &rsh_host->host_mutex );  
}


/*
  static const char * rsh_host_get_hostname(const rsh_host_type * host) { return host->host_name; }
*/



static void * rsh_host_submit_job__(void * __void_arg) {
  void_arg_type * void_arg = void_arg_safe_cast(__void_arg);
  char * rsh_cmd 	   = void_arg_get_ptr(void_arg , 0); 
  rsh_host_type * rsh_host = void_arg_get_ptr(void_arg , 1);
  char * submit_cmd 	   = void_arg_get_ptr(void_arg , 2); 
  char * run_path          = void_arg_get_ptr(void_arg , 3); 
  rsh_job_type * job       = void_arg_get_ptr(void_arg , 4);

  rsh_host_submit_job(rsh_host , job , rsh_cmd , submit_cmd , run_path);
  void_arg_free( void_arg );
  pthread_exit( NULL );

}



  

/*****************************************************************/


/*****************************************************************/

#define RSH_DRIVER_ID  1003
#define RSH_JOB_ID     2003

void rsh_driver_assert_cast(const rsh_driver_type * queue_driver) {
  if (queue_driver->__rsh_id != RSH_DRIVER_ID) 
    util_abort("%s: internal error - cast failed \n",__func__);
}

void rsh_driver_init(rsh_driver_type * queue_driver) {
  queue_driver->__rsh_id = RSH_DRIVER_ID;
}


void rsh_job_assert_cast(const rsh_job_type * queue_job) {
  if (queue_job->__rsh_id != RSH_JOB_ID) 
     util_abort("%s: internal error - cast failed \n",__func__);
}



rsh_job_type * rsh_job_alloc(int node_index , const char * run_path) {
  rsh_job_type * job;
  job = util_malloc(sizeof * job , __func__);
  job->__rsh_id   = RSH_JOB_ID;
  job->active     = false;
  job->running    = false; 
  job->run_path   = util_alloc_string_copy(run_path);
  job->node_index = node_index;
  return job;
}



void rsh_job_free(rsh_job_type * job) {
  free(job->run_path);
  free(job);
}



ecl_job_status_type rsh_driver_get_job_status(basic_queue_driver_type * __driver , basic_queue_job_type * __job) {
  if (__job == NULL) 
    /* The job has not been registered at all ... */
    return job_queue_null;
  else {
    rsh_job_type    * job    = (rsh_job_type    *) __job;
    rsh_driver_type * driver = (rsh_driver_type *) __driver;
    rsh_driver_assert_cast(driver); 
    rsh_job_assert_cast(job);
    {
      ecl_job_status_type status = -1; /* Dummy to shut up compiler warning. */
      if (job->active == false) 
	util_abort("%s: internal error - should not query status on inactive jobs \n" , __func__);
      else {
	if (job->running)
	  status = job_queue_running;
	else
	  status = job_queue_done;
      }
      return status;
    }
  }
}



void rsh_driver_free_job(basic_queue_driver_type * __driver , basic_queue_job_type * __job) {
  rsh_job_type    * job    = (rsh_job_type    *) __job;
  rsh_driver_type * driver = (rsh_driver_type *) __driver;
  rsh_driver_assert_cast(driver); 
  rsh_job_assert_cast(job);
  rsh_job_free(job);
}



void rsh_driver_abort_job(basic_queue_driver_type * __driver , basic_queue_job_type * __job) {
  rsh_job_type    * job    = (rsh_job_type    *) __job;
  rsh_driver_type * driver = (rsh_driver_type *) __driver;
  rsh_driver_assert_cast(driver); 
  rsh_job_assert_cast(job);
  if (job->active)
    pthread_kill(job->run_thread , SIGABRT);
  rsh_driver_free_job(__driver , __job);
}



basic_queue_job_type * rsh_driver_submit_job(basic_queue_driver_type * __driver, 
					     int   node_index , 
					     const char * submit_cmd  	  , 
					     const char * run_path    	  , 
					     const char * job_name) {
  
  rsh_driver_type * driver = (rsh_driver_type *) __driver;
  rsh_driver_assert_cast(driver); 
  {
    basic_queue_job_type * basic_job = NULL;
    /* 
       command is freed in the start_routine() function
    */
    rsh_host_type * host = NULL;
    int    ihost;
    pthread_mutex_lock( &driver->submit_lock );
    for (ihost = 0; ihost < driver->num_hosts; ihost++) {
      if (rsh_host_available(driver->host_list[ihost])) {
	host = driver->host_list[ihost];
	break;
      }
    }
    if (host != NULL) {
      /* A host is available */
      void_arg_type * void_arg = void_arg_alloc5( void_pointer , void_pointer , void_pointer , void_pointer , void_pointer);
      rsh_job_type  * job = rsh_job_alloc(node_index , run_path);
  
      void_arg_pack_ptr(void_arg , 0 ,  driver->rsh_command);
      void_arg_pack_ptr(void_arg , 1 ,  host);
      void_arg_pack_ptr(void_arg , 2 , (char *) submit_cmd);
      void_arg_pack_ptr(void_arg , 3 , (char *) run_path);
      void_arg_pack_ptr(void_arg , 4 , job);

      {
	int pthread_return_value = pthread_create( &job->run_thread , &driver->thread_attr , rsh_host_submit_job__ , void_arg);
	if (pthread_return_value != 0) 
	  util_abort("%s failed to create thread ERROR:%d  \n", __func__ , pthread_return_value);
      }
      job->active = true;
      basic_job = (basic_queue_job_type *) job;
      basic_queue_job_init(basic_job);
    } 
    pthread_mutex_unlock( &driver->submit_lock );

    return basic_job;
  }
}


/**
   The rsh_host_list should be a list of strings of the following
   format:

   rsh_host_list = ["host1:2",   "host2:2", "host4:4" ,  "host6:2"]

   i.e each host consists of a name and a number; the number
   designating how many concurrent jobs this host can handle. Observe
   that the load of the host is *not* consulted.
*/

void * rsh_driver_alloc(const char * rsh_command, int num_hosts , const char ** rsh_host_list) {
  rsh_driver_type * rsh_driver = util_malloc(sizeof * rsh_driver , __func__);
  rsh_driver->__rsh_id         = RSH_DRIVER_ID;
  pthread_mutex_init( &rsh_driver->submit_lock , NULL );
  pthread_attr_init( &rsh_driver->thread_attr );
  pthread_attr_setdetachstate( &rsh_driver->thread_attr , PTHREAD_CREATE_DETACHED );

  rsh_driver->rsh_command = util_alloc_string_copy(rsh_command);
  rsh_driver->submit      = rsh_driver_submit_job;
  rsh_driver->get_status  = rsh_driver_get_job_status;
  rsh_driver->abort_f     = rsh_driver_abort_job;
  rsh_driver->free_job    = rsh_driver_free_job;
  rsh_driver->free_driver = rsh_driver_free__;
  rsh_driver->num_hosts   = 0;
  rsh_driver->host_list   = NULL;
  {
    int ihost;
    for (ihost = 0; ihost < num_hosts; ihost++) {
      int pos = 0;
      while ( pos < strlen(rsh_host_list[ihost]) && rsh_host_list[ihost][pos] != ':') 
	pos++;

      {
	char *host = util_alloc_substring_copy(rsh_host_list[ihost] , pos);
	int max_running;

	if (pos == strlen(rsh_host_list[ihost])) {
	  fprintf(stderr," ** Warning no \":\" found for host:%s - assuming only one job to this host\n",host);
	  max_running = 1;
	} else 
	  if (!util_sscanf_int(&rsh_host_list[ihost][pos+1] , &max_running))
	    util_abort("%s: failed to parse integer from: %s - format should be host:number \n",__func__ , rsh_host_list[ihost]);
	
	rsh_driver_add_host(rsh_driver , host , max_running);
	free(host);
      }
    }
  }
    
  {
    basic_queue_driver_type * basic_driver = (basic_queue_driver_type *) rsh_driver;
    basic_queue_driver_init(basic_driver);

    return basic_driver;
  }
}


void rsh_driver_add_host(rsh_driver_type * rsh_driver , const char * hostname , int host_max_running) {
  rsh_host_type * new_host = rsh_host_alloc(hostname , host_max_running);
  if (new_host != NULL) {
    rsh_driver->num_hosts++;
    rsh_driver->host_list = util_realloc(rsh_driver->host_list , rsh_driver->num_hosts * sizeof * rsh_driver->host_list , __func__);
    rsh_driver->host_list[(rsh_driver->num_hosts - 1)] = new_host;
  }
}


void rsh_driver_free(rsh_driver_type * driver) {
  int ihost;
  for (ihost =0; ihost < driver->num_hosts; ihost++) 
    rsh_host_free(driver->host_list[ihost]);
  free(driver->host_list);

  pthread_attr_destroy ( &driver->thread_attr );
  free(driver);
  driver = NULL;
}


void rsh_driver_free__(basic_queue_driver_type * driver) {
  rsh_driver_free((rsh_driver_type *) driver);
}



#undef RSH_DRIVER_ID  
#undef RSH_JOB_ID    

/*****************************************************************/

