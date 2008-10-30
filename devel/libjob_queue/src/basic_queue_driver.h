#ifndef __BASIC_QUEUE_DRIVER_H__
#define __BASIC_QUEUE_DRIVER_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {job_queue_null          = 0 ,   /* For a queue node which has been allocated - but not "added" with a job_queue_add_job() call. */
	      job_queue_waiting       = 1 ,   /* A node which is waiting in the internal queue. */
	      job_queue_pending       = 2 ,   /* A node which is pending - a status returned by the external system. I.e LSF */
	      job_queue_running       = 3 ,   /* The job is running */
	      job_queue_done          = 4 ,   /* The job is done - but we have not yet checked if the target file is produced */
	      job_queue_exit          = 5 ,   /* The job has exited - check attempts to determine if we retry or go to complete_fail */
	      job_queue_complete_OK   = 6 ,   
	      job_queue_complete_FAIL = 7 ,
	      job_queue_restart       = 8 ,
	      job_queue_max_state     = 9 } job_status_type;


typedef struct basic_queue_driver_struct basic_queue_driver_type;
typedef struct basic_queue_job_struct    basic_queue_job_type;

typedef basic_queue_job_type * (submit_job_ftype)  	    (basic_queue_driver_type * , int , const char * , const char * , const char * );
typedef void                   (abort_job_ftype)   	    (basic_queue_driver_type * , basic_queue_job_type * );
typedef job_status_type        (get_status_ftype)  	    (basic_queue_driver_type * , basic_queue_job_type * );
typedef void                   (free_job_ftype)    	    (basic_queue_driver_type * , basic_queue_job_type * );
typedef void                   (free_queue_driver_ftype)    (basic_queue_driver_type *);
typedef void                   (set_resource_request_ftype) (basic_queue_driver_type * , const char *); 

void basic_queue_driver_assert_cast(const basic_queue_driver_type * );
void basic_queue_driver_init(basic_queue_driver_type * );
void basic_queue_job_assert_cast(const basic_queue_job_type * );
void basic_queue_job_init(basic_queue_job_type * );


struct basic_queue_job_struct {
  int __id;
};

#define BASIC_QUEUE_DRIVER_FIELDS           	    \
submit_job_ftype  	   * submit;        	    \
free_job_ftype    	   * free_job;      	    \
abort_job_ftype   	   * abort_f;       	    \
get_status_ftype  	   * get_status;    	    \
free_queue_driver_ftype    * free_driver;   	    \
set_resource_request_ftype * set_resource_request;  \
int __id;        


struct basic_queue_driver_struct {
  BASIC_QUEUE_DRIVER_FIELDS
};




#ifdef __cplusplus
}
#endif
#endif
