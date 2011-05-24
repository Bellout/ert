#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'ecl_grid.py' is part of ERT - Ensemble based Reservoir Tool. 
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

"""
Module to load and query ECLIPSE GRID/EGRID files.

The ecl_grid module contains functionality to load and query an
ECLIPSE grid file; it is currently not possible to manipulate or let
alone create a grid with ecl_grid module. The functionality is
implemented in the EclGrid class. The ecl_grid module is a thin
wrapper around the ecl_grid.c implementation from the libecl library.
"""


import ctypes
from   ert.cwrap.cwrap       import *
from   ert.util.tvector      import DoubleVector  # Requires merging of typemaps ....
import numpy
import libecl
import ecl_kw
from   ert.util.cfile        import CFILE

class EclGrid(object):
    
    @classmethod
    def create(cls , specgrid , zcorn , coord , actnum , mapaxes = None ):
        """
        Create a new grid instance from existing keywords.

        This is a class method which can be used to create an EclGrid
        instance based on the EclKW instances @specgrid, @zcorn,
        @coord and @actnum. An ECLIPSE EGRID file contains the
        SPECGRID, ZCORN, COORD and ACTNUM keywords, so a somewhat
        involved way to create a EclGrid instance could be:

          file = ecl.EclFile( "ECLIPSE.EGRID" )
          specgrid_kw = file.iget_named_kw( "SPECGRID" , 0)
          zcorn_kw = file.iget_named_kw( "ZCORN" , 0)
          coord_kw = file.iget_named_kw( "COORD" , 0)
          actnum_kw = file.iget_named_kw( "ACTNUM" , 0 )
          
          grid = EclGrid.create( specgrid_kw , zcorn_kw , coord_kw , actnum_kw)
          
        If you are so inclined ...  
        """
        obj = object.__new__( cls )
        gridhead = ecl_kw.EclKW.new( "GRIDHEAD" , 4 , 3 ) 
        gridhead[0] = 1 
        for i in (range(3)): 
            gridhead[i+1] = specgrid[i]
    
        obj.c_ptr = cfunc.grdecl_create( gridhead , zcorn , coord , actnum , None) # , mapaxes) 
        obj.data_owner = True
        obj.parent     = None
        return obj


    def __new__(cls , filename , lgr = None , parent = None):
        if filename:
            c_ptr = cfunc.fread_alloc( filename )
        elif lgr:
            c_ptr = lgr
            
        if c_ptr:
            obj = object.__new__( cls )
            obj.c_ptr = c_ptr
            if lgr:
                obj.data_owner = False
                obj.parent     = parent    # Keep a reference to the parent to inhibit GC.
            else:
                obj.data_owner = True
                obj.parent     = None
            return obj
        else:
            return None

    def __del__(self):
        if self.data_owner:
            cfunc.free( self )

    def from_param(self):
        """
        ctypes utility function.
        
        ctypes utility method facilitating transparent mapping between
        python EclGrid instances and C based ecl_grid_type pointers.
        """
        return self.c_ptr

    @property
    def nx( self ):
        """The number of cells in the i direction - nx."""
        return cfunc.get_nx( self )

    @property
    def ny( self ):
        """The number of cells in the j direction - ny."""
        return cfunc.get_ny( self )

    @property
    def nz( self ):
        """The number of cells in the k direction - nz."""
        return cfunc.get_nz( self )

    @property
    def size( self ):
        """The total number of cells in the grid, i.e. nx*ny*nz."""
        return cfunc.get_nx( self ) * cfunc.get_ny( self ) * cfunc.get_nz( self )

    @property
    def nactive( self ):
        """The number of active cells in the grid."""
        return cfunc.get_active( self )

    @property
    def dims( self ):
        """A tuple of four elements: (nx , ny , nz , nactive)."""
        return ( cfunc.get_nx( self ) ,
                 cfunc.get_ny( self ) ,
                 cfunc.get_nz( self ) ,
                 cfunc.get_active( self ) )

    @property
    def name( self ):
        """
        Name of the current grid.
        
        For the main grid this is the filename given to the
        constructor when loading the grid; for an LGR this is the name
        of the LGR. If the grid instance has been created with the
        create() classmethod this can be None.
        """
        return cfunc.get_name( self )

    def __global_index( self , active_index = None , global_index = None , ijk = None):
        """
        Will convert @active_index or @ijk to global_index.

        This method will convert @active_index or @ijk to a global
        index. Exactly one of the arguments @active_index,
        @global_index or @ijk must be supplied. 

        The method is used extensively internally in the EclGrid
        class; most methods which take coordinate input pass through
        this method to normalize the coordinate representation.
        """

        set_count = 0
        if not active_index == None:
            set_count += 1
        if not global_index == None:
            set_count += 1
        if ijk:
            set_count += 1
            
        if not set_count == 1:
            raise ValueError("Exactly one of the kewyord arguments active_index, global_index or ijk must be set")
        
        if active_index:
            global_index = cfunc.get_global_index1A( self , active_index )
        elif ijk:
            global_index = cfunc.get_global_index3( self , ijk[0] , ijk[1] , ijk[2])
        
        return global_index
                 

    def get_active_index( self , ijk = None , global_index = None):
        """
        Lookup active index based on ijk or global index.

        Will determine the active_index of a cell, based on either
        @ijk = (i,j,k) or @global_index. If the cell specified by the
        input arguments is not active the function will return -1.
        """
        gi = self.__global_index( global_index = global_index , ijk = ijk)
        return cfunc.get_active_index1( self , gi)

    
    def active( self , ijk = None , global_index = None):
        """
        Is the cell active?

        See documentation og get_xyz() for explanation of parameters
        @ijk and @global_index.
        """
        gi = self.__global_index( global_index = global_index , ijk = ijk)
        active_index = cfunc.get_active_index1( self , gi)
        if active_index >= 0:
            return True
        else:
            return False


    def get_global_index( self , ijk = None , active_index = None):
        """
        Lookup global index based on ijk or active index.
        """
        gi = self.__global_index( active_index = active_index , ijk = ijk)
        return gi


    def get_ijk( self, active_index = None , global_index = None):
        """
        Lookup (i,j,k) for a cell, based on either active index or global index.

        The return value is a tuple with three elements (i,j,k).
        """
        i = ctypes.c_int()
        j = ctypes.c_int()
        k = ctypes.c_int()

        gi = self.__global_index( active_index = active_index , global_index = global_index)
        cfunc.get_ijk1( self , gi , ctypes.byref(i) , ctypes.byref(j) , ctypes.byref(k))

        return (i.value , j.value , k.value)


    def get_xyz( self, active_index = None , global_index = None , ijk = None):
        """
        Find true position of cell center.

        Will return world position, i.e. (utm_x , utm_y , depth), of
        the center of a cell in the grid.
        
        The cells of a grid can be specified in three different ways:

           (i,j,k)      : As a tuple of i,j,k values.

           global_index : A number in the range [0,nx*ny*nz). The
                          global index is related to (i,j,k) as:

                            global_index = i + j*nx + k*nx*ny
           
           active_index : A number in the range [0,nactive).
           
        For many of the EclGrid methods a cell can be specified using
        any of these three methods. Observe that one and only method is
        allowed:

        OK:
            pos1 = grid.get_xyz( active_index = 100 )                    
            pos2 = grid.get_xyz( ijk = (10,20,7 ))                       

        Crash and burn:
            pos3 = grid.get_xyz( ijk = (10,20,7 ) , global_index = 10)   
            pos4 = grid.get_xyz()
            
        All the indices in the EclGrid() class are zero offset, this
        is in contrast to ECLIPSE which has an offset 1 interface.
        """
        gi = self.__global_index( ijk = ijk , active_index = active_index , global_index = global_index)

        x = ctypes.c_double()
        y = ctypes.c_double()
        z = ctypes.c_double()

        cfunc.get_xyz1( self , gi , ctypes.byref(x) , ctypes.byref(y) , ctypes.byref(z))
        return (x.value , y.value , z.value)

    def depth( self , active_index = None , global_index = None , ijk = None):
        """
        Depth of the center of a cell.

        Returns the depth of the center of the cell given by
        @active_index, @global_index or @ijk. See method get_xyz() for
        documentation of @active_index, @global_index and @ijk.
        """
        gi = self.__global_index( ijk = ijk , active_index = active_index , global_index = global_index)
        return cfunc.get_depth( self , gi )

    def top( self , i , j ):
        """
        Top of the reservoir; in the column (@i , @j).
        """
        return cfunc.get_top( self , i , j ) 

    def bottom( self , i , j ):
        """
        Bottom of the reservoir; in the column (@i , @j).
        """
        return cfunc.get_bottom( self , i , j ) 

    def locate_depth( self , depth , i , j ):
        """
        Will locate the k value of cell containing specified depth.

        Will scan through the grid column specified by the input
        arguments @i and @j and search for a cell containing the depth
        given by input argument @depth. The return value is the k
        value of cell containing @depth.

        If @depth is above the top of the reservoir the function will
        return -1, and if @depth is below the bottom of the reservoir
        the function will return -nz.
        """
        return cfunc.locate_depth( self , depth , i , j)

    def find_cell( self , x , y , z , start_ijk = None):
        """
        Lookup cell containg true position (x,y,z).

        Will locate the cell in the grid which contains the true
        position (@x,@y,@z), the return value is as a triplet
        (i,j,k). The underlying C implementation is not veeery
        efficient, and can potentially take quite long time. If you
        provide a good intial guess with the parameter @start_ijk (a
        tuple (i,j,k)) things can speed up quite substantially.

        If the location (@x,@y,@z) can not be found in the grid, the
        method will return None.
        """

        if start_ijk:
            start_index = self.__global_index( ijk = start_ijk )
        else:
            start_index = 0
        global_index = cfunc.get_ijk_xyz( self , x , y , z , start_index)
        if global_index >= 0:
            i = ctypes.c_int()
            j = ctypes.c_int()
            k = ctypes.c_int()
            cfunc.get_ijk1( self , global_index , ctypes.byref(i) , ctypes.byref(j) , ctypes.byref(k))        
            return (i.value , j.value , k.value)
        else:
            return None

    def cell_volume( self, active_index = None , global_index = None , ijk = None):
        """
        Calculate the volume of a cell.

        Will calculate the total volume of the cell. See method
        get_xyz() for documentation of @active_index, @global_index
        and @ijk.
        """
        gi = self.__global_index( ijk = ijk , active_index = active_index , global_index = global_index)
        return cfunc.get_cell_volume( self , gi)
            

    def cell_dz( self , active_index = None , global_index = None , ijk = None):
        """
        The thickness of a cell.

        Will calculate the (average) thickness of the cell. See method
        get_xyz() for documentation of @active_index, @global_index
        and @ijk.
        """
        gi = self.__global_index( ijk = ijk , active_index = active_index , global_index = global_index )
        return cfunc.get_cell_thickness( self , gi )


    @property
    def num_lgr( self ):
        """
        How many LGRs are attached to this main grid?

        How many LGRs are attached to this main grid; the grid
        instance doing the query must itself be a main grid.
        """
        return cfunc.num_lgr( self )


    def has_lgr( self , lgr_name ):
        """
        Query if the grid has an LGR with name @lgr_name.
        """
        if cfunc.has_lgr( self , lgr_name ):
            return True
        else:
            return False


    def get_lgr( self , lgr_name ):
        """
        Get EclGrid instance with LGR content.
        
        Return an EclGrid instance based on the LGR named
        @lgr_name. The LGR grid instance is in most questions like an
        ordinary grid instance; the only difference is that it can not
        be used for further queries about LGRs.

        If the grid does not contain an LGR with this name the method
        will return None.
        """
        if cfunc.has_lgr(self , lgr_name ):
            lgr = EclGrid( None , lgr = cfunc.get_lgr( self , lgr_name ) , parent = self)
            return lgr
        else:
            return None
        

    def get_cell_lgr( self, active_index = None , global_index = None , ijk = None):
        """
        Get EclGrid instance located in cell.
        
        Will query the current grid instance if the cell given by
        @active_index, @global_index or @ijk has been refined with an
        LGR. Will return None if the cell in question has not been
        refined, the return value can be used for further queries.
        
        See get_xyz() for documentation of the input parameters.
        """
        gi  = self.__global_index( ijk = ijk , active_index = active_index , global_index = global_index)
        lgr = cfunc.get_cell_lgr( self , gi )
        if lgr:
            return EclGrid( None , lgr = lgr , parent = self)
        else:
            return None

    
    def grid_value( self , kw , i , j , k):
        """
        Will evalute @kw in location (@i,@j,@k).

        The ECLIPSE properties and solution vectors are stored in
        restart and init files as 1D vectors of length nx*nx*nz or
        nactive. The grid_value() method is a minor convenience
        function to convert the (@i,@j,@k) input values to an
        appropriate 1D index.

        Depending on the length of kw the input arguments are
        converted either to an active index or to a global index. If
        the length of kw does not fit with either the global size of
        the grid or the active size of the grid things will fail hard.
        """
        return cfunc.grid_value( self , kw , i , j , k)


    def load_column( self , kw , i , j , column):
        """
        Load the values of @kw from the column specified by (@i,@j).

        The method will scan through all k values of the input field
        @kw for fixed values of i and j. The size of @kw must be
        either nactive or nx*ny*nz.

        The input argument @column should be a DoubleVector instance,
        observe that if size of @kw == nactive k values corresponding
        to inactive cells will not be modified in the @column
        instance; in that case it is important that @column is
        initialized with a suitable default value.
        """
        cfunc.load_column( self , kw , i , j , column)
    

    def createKW( self , array , kw_name , pack):
        """
        Creates an EclKW instance based on existing 3D numpy object.

        The methods create3D() does the inverse operation; creating a
        3D numpy object from an EclKW instance.
        """
        if array.ndim == 3:
            dims = array.shape
            if dims[0] == self.nx and dims[1] == self.ny and dims[2] == self.nz:
                dtype = array.dtype
                if dtype == numpy.int32:
                    type = ecl_kw.ECL_INT_TYPE
                elif dtype == numpy.float32:
                    type = ecl_kw.ECL_FLOAT_TYPE
                elif dtype == numpy.float64:
                    type = ecl_kw.ECL_DOUBLE_TYPE
                else:
                    sys.exit("Do not know how to create ecl_kw from type:%s" % dtype)
  
                if pack:
                    size = self.nactive
                else:
                    size = self.size
                    
                if len(kw_name) > 8:
                    # Silently truncate to length 8 - ECLIPSE has it's challenges.
                    kw_name = kw_name[0:8]  

                kw = ecl_kw.EclKW.new( kw_name , size , type )
                active_index = 0
                global_index = 0
                for k in range( self.nz ):
                    for j in range( self.ny ):
                        for i in range( self.nx ):
                            if pack:
                                if self.active( global_index = global_index ):
                                    kw[active_index] = array[i,j,k]
                                    active_index += 1
                            else:
                                if dtype == numpy.int32:
                                    kw[global_index] = int( array[i,j,k] )
                                else:
                                    kw[global_index] = array[i,j,k]
                                
                            global_index += 1
                return kw
        raise ValueError("Wrong size / dimension on array")

    
    

    def create3D( self , ecl_kw , default = 0):
        """
        Creates a 3D numpy array object with the data from  @ecl_kw.

        Observe that 3D numpy object is a copy of the data in the
        EclKW instance, i.e. modification to the numpy object will not
        be reflected in the ECLIPSE keyword.
        
        The methods createKW() does the inverse operation; creating an
        EclKW instance from a 3D numpy object.
        """
        if ecl_kw.size == self.nactive or ecl_kw.size == self.size:
            array = numpy.ones( [ self.nx , self.ny , self.nz] , dtype = ecl_kw.dtype) * default
            array = numpy.ones( [ self.size ] , dtype = ecl_kw.dtype) * default
            kwa = ecl_kw.array
            if ecl_kw.size == self.size:
                for i in range(kwa.size):
                    array[i] = kwa[i]
            else:
                data_index = 0
                for global_index in range(self.size):
                    if self.active( global_index = global_index ):
                        array[global_index] = kwa[data_index]
                        data_index += 1
                        
            array = array.reshape( [self.nx , self.ny , self.nz] , order = 'F')
            return array
        else:
            raise ValueError("Keyword: %s has invalid size(%d), must be either nactive:%d  or nx*ny*nz:%d" % (ecl_kw.name , ecl_kw.size , self.nactive ,self.size))
        
        
    def write_grdecl( self , ecl_kw , pyfile , default_value = 0):
        """
        Writes an EclKW instance as an ECLIPSE grdecl formatted file.

        The input argument @ecl_kw must be an EclKW instance of size
        nactive or nx*ny*nz. If the size is nactive the inactive cells
        will be filled with @default_value; hence the function will
        always write nx*ny*nz elements. 
        
        The data in the @ecl_kw argument can be of type integer,
        float, double or bool. In the case of bool the default value
        must be specified as 1 (True) or 0 (False).

        The input argument @pyfile should be a valid python filehandle
        opened for writing; i.e.

           pyfile = open("PORO.GRDECL" , "w")
           grid.write_grdecl( poro_kw  , pyfile , default_value = 0.0)
           grid.write_grdecl( permx_kw , pyfile , default_value = 0.0)
           pyfile.close()

        """
        
        if ecl_kw.size == self.nactive or ecl_kw.size == self.size:
            cfile = CFILE( pyfile )
            cfunc.fwrite_grdecl( self , ecl_kw , cfile , default_value )
        else:
            raise ValueError("Keyword: %s has invalid size(%d), must be either nactive:%d  or nx*ny*nz:%d" % (ecl_kw.name , ecl_kw.size , self.nactive , self.size))



# 2. Creating a wrapper object around the libecl library, 
#    registering the type map : ecl_kw <-> EclKW
cwrapper = CWrapper( libecl.lib )
cwrapper.registerType( "ecl_grid" , EclGrid )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("ecl_grid")



cfunc.fread_alloc                  = cwrapper.prototype("c_void_p ecl_grid_load_case( char* )")
cfunc.grdecl_create                = cwrapper.prototype("c_void_p ecl_grid_alloc_GRDECL_kw( ecl_kw , ecl_kw , ecl_kw , ecl_kw )") # MAPAXES not supported yet
cfunc.get_lgr                      = cwrapper.prototype("c_void_p ecl_grid_get_lgr( ecl_grid , char* )")
cfunc.get_cell_lgr                 = cwrapper.prototype("c_void_p ecl_grid_get_cell_lgr1( ecl_grid , int )")
cfunc.exists                       = cwrapper.prototype("bool ecl_grid_exists( char* )")
cfunc.free                         = cwrapper.prototype("void ecl_grid_free( ecl_grid )")     
cfunc.get_nx                       = cwrapper.prototype("int ecl_grid_get_nx( ecl_grid )")
cfunc.get_ny                       = cwrapper.prototype("int ecl_grid_get_ny( ecl_grid )")
cfunc.get_nz                       = cwrapper.prototype("int ecl_grid_get_nz( ecl_grid )")
cfunc.get_active                   = cwrapper.prototype("int ecl_grid_get_active_size( ecl_grid )")
cfunc.get_name                     = cwrapper.prototype("char* ecl_grid_get_name( ecl_grid )")
cfunc.get_active_index3            = cwrapper.prototype("int ecl_grid_get_active_index3( ecl_grid , int , int , int)")
cfunc.get_global_index3            = cwrapper.prototype("int ecl_grid_get_global_index3( ecl_grid , int , int , int)") 
cfunc.get_active_index1            = cwrapper.prototype("int ecl_grid_get_active_index1( ecl_grid , int )") 
cfunc.get_global_index1A           = cwrapper.prototype("int ecl_grid_get_global_index1A( ecl_grid , int )") 
cfunc.get_ijk1                     = cwrapper.prototype("void ecl_grid_get_ijk1( ecl_grid , int , int* , int* , int*)")
cfunc.get_ijk1A                    = cwrapper.prototype("void ecl_grid_get_ijk1A( ecl_grid , int , int* , int* , int*)") 
cfunc.get_xyz3                     = cwrapper.prototype("void ecl_grid_get_xyz3( ecl_grid , int , int , int , double* , double* , double*)")
cfunc.get_xyz1                     = cwrapper.prototype("void ecl_grid_get_xyz1( ecl_grid , int , double* , double* , double*)")
cfunc.get_xyz1A                    = cwrapper.prototype("void ecl_grid_get_xyz1A( ecl_grid , int , double* , double* , double*)")
cfunc.get_ijk_xyz                  = cwrapper.prototype("int  ecl_grid_get_global_index_from_xyz( ecl_grid , double , double , double , int)")
cfunc.num_lgr                      = cwrapper.prototype("int  ecl_grid_get_num_lgr( ecl_grid )")
cfunc.has_lgr                      = cwrapper.prototype("bool ecl_grid_has_lgr( ecl_grid , char* )")
cfunc.grid_value                   = cwrapper.prototype("double ecl_grid_get_property( ecl_grid , ecl_kw , int , int , int)")
cfunc.get_cell_volume              = cwrapper.prototype("double ecl_grid_get_cell_volume1( ecl_grid , int )")
cfunc.get_cell_thickness           = cwrapper.prototype("double ecl_grid_get_cell_thickness1( ecl_grid , int )")
cfunc.get_depth                    = cwrapper.prototype("double ecl_grid_get_cdepth1( ecl_grid , int )")
cfunc.fwrite_grdecl                = cwrapper.prototype("void   ecl_grid_grdecl_fprintf_kw( ecl_grid , ecl_kw , FILE , double)") 
cfunc.load_column                  = cwrapper.prototype("void   ecl_grid_get_column_property( ecl_grid , ecl_kw , int , int , double_vector)")
cfunc.get_top                      = cwrapper.prototype("double ecl_grid_get_top2( ecl_grid , int , int )") 
cfunc.get_bottom                   = cwrapper.prototype("double ecl_grid_get_bottom2( ecl_grid , int , int )") 
cfunc.locate_depth                 = cwrapper.prototype("int    ecl_grid_locate_depth( ecl_grid , double , int , int )") 
