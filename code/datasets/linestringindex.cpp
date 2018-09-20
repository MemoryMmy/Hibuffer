/*
* 
*    data: 2018-08-10 
*    auther : Ma Mengyu@DBRG
*    description : create indexes(Linestring objects)
*    dependency :
*    gdal2.10  boost1.64 
*    run :
* 	 $  ./Linestringindex --shp ./datasets/alaska_OSM/gis_osm_roads_free_1.shp --output ./indexes/lindex --size 69
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
	//~ typedef bgm::box<point> box;
	typedef bgm::segment<point> segment;

    
    struct timeval t1,t2;
    double timeuse;
    gettimeofday(&t1,NULL);
	

	
    typedef bgi::quadratic<MAX_NODE_SIZE> params;
    typedef bgi::indexable<segment> indexable_segment;
    typedef bgi::equal_to<segment> equal_to_segment;
    typedef bi::allocator<segment, bi::managed_mapped_file::segment_manager> allocator_segment;
    typedef bgi::rtree<segment, params, indexable_segment, equal_to_segment, allocator_segment> rtree_segment;
	

	const char* shpFile = NULL;     
	const char* outIndex=NULL;
	long size =0;
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
	
	bi::managed_mapped_file file(bi::create_only, outIndex, size*1014*1024);
    allocator_segment alloc(file.get_segment_manager());
    rtree_segment * rtree_ptr = file.construct<rtree_segment>("rtree")(params(), indexable_segment(), equal_to_segment(), alloc);
	
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
	
	
	double px0,py0,px1,py1;
	while((shpFeature=shpLayer-> GetNextFeature())!= NULL)
	{	
		OGRGeometry *poGeometry=shpFeature->GetGeometryRef();
		int eType = wkbFlatten(poGeometry->getGeometryType());
		if(eType == wkbLineString)
		{	
			OGRLineString* pOGRLineString=(OGRLineString*) poGeometry;
			int pointCount = pOGRLineString->getNumPoints();
			px0=pOGRLineString->getX(0);
			py0=pOGRLineString->getY(0);
			coordTrans ->Transform(1,&px0,&py0);
			for(int i=1;i<pointCount;i++)
			{
				px1=pOGRLineString->getX(i);
				py1=pOGRLineString->getY(i);
				coordTrans ->Transform(1,&px1,&py1);
				rtree_ptr->insert(segment(point(px0, py0),point(px1,py1)));
				px0=px1;
				py0=py1;
			}	
		}
		else if( eType == wkbMultiLineString)
		{
			OGRMultiLineString* pOGRMultiLineString=(OGRMultiLineString*) poGeometry;
			int lineCount =pOGRMultiLineString->getNumGeometries();
			for(int j=0;j<lineCount;j++)
			{
				OGRLineString* pOGRLineString=(OGRLineString*)pOGRMultiLineString->getGeometryRef(j);
				int pointCount = pOGRLineString->getNumPoints();
				px0=pOGRLineString->getX(0);
				py0=pOGRLineString->getY(0);
				coordTrans ->Transform(1,&px0,&py0);
				for(int i=1;i<pointCount;i++)
				{
					px1=pOGRLineString->getX(i);
					py1=pOGRLineString->getY(i);
					coordTrans ->Transform(1,&px1,&py1);
					rtree_ptr->insert(segment(point(px0, py0),point(px1,py1)));
					px0=px1;
					py0=py1;
				}
			}		
		}
	}
	std::cout << rtree_ptr->size() << std::endl;
	gettimeofday(&t2,NULL);
    timeuse=t2.tv_sec-t1.tv_sec+(t2.tv_usec-t1.tv_usec)/1000000.0;
    printf("Use Time:%f\n",timeuse);
}
