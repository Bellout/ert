#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'forward_model.py' is part of ERT - Ensemble based Reservoir Tool. 
#   
#  ERT is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or 
#  (at your option) any later version. 
#   
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.   
#   
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
#  for more details. 

import  ctypes
from    ert.cwrap.cwrap       import *
from    ert.cwrap.cclass      import CClass
from    ert.util.tvector      import * 
import  libjob_queue
class ForwardModel(CClass):
    
    def __init__(self , c_ptr = None):
        self.owner = False
        self.c_ptr = c_ptr
        
        
    def __del__(self):
        if self.owner:
            cfunc.free( self )

    @property
    def alloc_joblist(self):
        return cfunc.alloc_joblist(self)

    def iget_job(self, index):
        job = ert.job_queue.ext_job.ExtJob( cfunc.iget_job( self, index ))
        return job

    def add_job(self, name):
        job = ert.job_queue.ext_job.ExtJob( cfunc.iget_job( self, name ))
        return job

##################################################################

cwrapper = CWrapper( libjob_queue.lib )
cwrapper.registerType( "forward_model" , ForwardModel )

cfunc = CWrapperNameSpace("forward_model")
##################################################################
##################################################################
cfunc.free                       = cwrapper.prototype("void forward_model_free( forward_model )")
cfunc.clear                      = cwrapper.prototype("void forward_model_clear(forward_model)")
cfunc.add_job                    = cwrapper.prototype("c_void_p forward_model_add_job(forward_model, char*)")
cfunc.alloc_joblist              = cwrapper.prototype("c_void_p forward_model_alloc_joblist(forward_model)")
cfunc.iget_job                   = cwrapper.prototype("c_void_p forward_model_iget_job( forward_model, int)")
