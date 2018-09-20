/*
* 
*    data: 2018-08-10 
*    auther : Ma Mengyu@DBRG
*    description :
*    create indexes(Point objects)
*    dependency :
*    gdal2.10  boost1.64 
*    run :
* 	 $  ./Pointindex --shp ./datasets/alaska_OSM/gis_osm_pois_free_1.shp --output ./indexes/pindex --size 1
*/


#include "ogrsf_frmts.h"
#include "ogr_p.h"
#include "cpl_conv.h"
#include "cpl_string.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <dirent.h>
#include <omp.h>
#include <sys/time.h>
#include <regex.h>
#include <math.h>

#include <queue>
#include <utility>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/foreach.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#define MAX_NODE_SIZE 8
#define MAX_DOUBLE 1000000000

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
namespace bgm = boost::geometry::model;
namespace bi = boost::interprocess;

void GetFilelist(char* argv, char* result[], char* flag, int& count)
{
	char* string = strdup(argv); 
	char* p;
	int i = 0;
	while((p = strsep(&string, flag)) != NULL)
	{
		result[i] = p;
		i++;
	}
	result[i] = string;
	count = i;
}

void Usage()
{
    printf( "Usage:           [--shp:       input shapefile     ]\n"
	    "                     [--output:    output indexes      ]\n"
	    "                     [--size:    index file size(MB)   ]\n"	    	    
	    );
}



int main( int nArgc, char ** papszArgv )
{
	typedef bgm::d2::point_xy<double> point;
    
    struct timeval t1,t2;
    double timeuse;
    gettimeofday(&t1,NULL);

    typedef bgi::quadratic<MAX_NODE_SIZE> params_t;
    typedef bgi::indexable<point> indexable_t;
    typedef bgi::equal_to<point> equal_to_t;
    typedef bi::allocator<point, bi::managed_mapped_file::segment_manager> allocator_t;
    typedef bgi::rtree<point, params_t, indexable_t, equal_to_t, allocator_t> rtree_t;
    
	const char* shpFile = NULL;     
	const char* outIndex=NULL;
	long size=0;
	double minXOut=MAX_DOUBLE,minYOut=MAX_DOUBLE,maxXout=-1*MAX_DOUBLE,maxYOut=-1*MAX_DOUBLE; 
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	CPLSetConfigOption("SHAPE_ENCODING", "UTF-8");
	for( int iArg = 1; iArg < nArgc; iArg++ )
	{
		if( EQUAL(papszArgv[iArg], "--shp") )
	    {
			shpFile = papszArgv[iArg+1];
			
		}
		else if( EQUAL(papszArgv[iArg], "--output") )
	    {
			outIndex = papszArgv[iArg+1];
		}else if( EQUAL(papszArgv[iArg], "--size") )
	    {
			size = atol(papszArgv[iArg+1]);
		}
	}
	
	if (shpFile == NULL || outIndex==NULL ||size==0)
	{
		Usage();
		return 0;
	}
	printf("--shp: %s\n", shpFile);
	printf("--output: %s\n", outIndex);
	printf("--size: %ld MB\n", size);
	
	bi::managed_mapped_file file(bi::create_only, outIndex, size*1024*1024);
	allocator_t alloc(file.get_segment_manager());
	rtree_t * rtree_ptr = file.find_or_construct<rtree_t>("rtree")(params_t(), indexable_t(), equal_to_t(), alloc);
	
	std::cout << rtree_ptr->size() << std::endl;

	OGRRegisterAll();
	OGRLayer* shpLayer;
	OGRFeature *shpFeature;
	GDALDataset* shpDS =(GDALDataset*)GDALOpenEx(shpFile, GDAL_OF_VECTOR,NULL, NULL, NULL );
	if(shpDS==NULL)
	{
		printf("[ERROR] Open shpFile failed.\n");
		return 0;
	}
	shpLayer=shpDS->GetLayer(0);
	
	OGREnvelope env;
	shpLayer->GetExtent(&env);
	minXOut=minXOut<env.MinX? minXOut:env.MinX;
	minYOut=minYOut<env.MinY? minYOut:env.MinY;
	maxXout=maxXout>env.MaxX? maxXout:env.MaxX;
	maxYOut=maxYOut>env.MaxY? maxYOut:env.MaxY;
	shpLayer->ResetReading();

	OGRSpatialReference* fRef;
	OGRSpatialReference tRef;
	fRef = shpLayer->GetSpatialRef();
	tRef.importFromEPSG(3857);
	OGRCoordinateTransformation *coordTrans;
	coordTrans = OGRCreateCoordinateTransformation(fRef, &tRef);
	
    coordTrans ->Transform(1,&minXOut,&minYOut);
    coordTrans ->Transform(1,&maxXout,&maxYOut);
    printf("minXOut: %lf,minYOut: %lf, maxXout %lf,maxYOut %lf\n", minXOut,minYOut, maxXout,maxYOut);
	

	double px0,py0;
	while((shpFeature=shpLayer-> GetNextFeature())!= NULL)
	{	
		OGRGeometry *poGeometry=shpFeature->GetGeometryRef();
		int eType = wkbFlatten(poGeometry->getGeometryType());
		if( eType == wkbPoint)
		{
			OGRPoint* pOGRPoint=(OGRPoint*) poGeometry;
			px0 = pOGRPoint->getX();
			py0 = pOGRPoint->getY();
			coordTrans ->Transform(1,&px0,&py0);
			rtree_ptr->insert(point(px0,py0));
		}else if( eType == wkbMultiPoint)
		{
			OGRMultiPoint* pOGRMultiPoint=(OGRMultiPoint*) poGeometry;
			int pointCount =pOGRMultiPoint->getNumGeometries();
			for(int j=0;j<pointCount;j++)
			{
				OGRPoint* pOGRPoint=(OGRPoint*)pOGRMultiPoint->getGeometryRef(j);
				px0 = pOGRPoint->getX();
				py0 = pOGRPoint->getY();
				coordTrans ->Transform(1,&px0,&py0);
				rtree_ptr->insert(point(px0,py0));
			}		
		}
	}
	std::cout << rtree_ptr->size() << std::endl;
	gettimeofday(&t2,NULL);
    timeuse=t2.tv_sec-t1.tv_sec+(t2.tv_usec-t1.tv_usec)/1000000.0;
    printf("Use Time:%f\n",timeuse);
}
