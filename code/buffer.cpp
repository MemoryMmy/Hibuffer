/*
* 
*    data: 2018-08-10 
*    author: Mengyu Ma@National University of Defense Technology
*    e-mail: mamengyu10@nudt.edu.cn
*    description: Buffer Tile Rendering Engine of HiBuffer 
*    run: 
* 	 $ mpirun -np 32 ./Buffer
* 	 
*/


#include "ogrsf_frmts.h"
#include "ogr_p.h"
#include "cpl_conv.h"
#include "cpl_string.h"
#include <stdio.h>
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <dirent.h>
#include <omp.h>
#include <sys/time.h>
#include <regex.h>
#include <math.h>
#include <mpi.h>
#include <stdlib.h>
#include <hiredis/hiredis.h>
#include <queue>
#include <utility>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/foreach.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>

#include <png.h> 
#include <stdlib.h>



#define MAX_NODE_SIZE 8
#define MAX_TILE_PARAMS 256
#define L 20037508.34

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
namespace bgm = boost::geometry::model;
namespace bi = boost::interprocess;

using namespace std;

class Redis
{
	public:

    	Redis(){}
	
	~Redis()
	{
		this->_connect = NULL;
		this->_reply = NULL;	    	    
	}

	bool connect(string host, int port)
	{
		this->_connect = redisConnect(host.c_str(), port);
		if (this->_connect != NULL && this->_connect->err)
		{
			printf("connect error: %s\n", this->_connect->errstr);
			return false;
		}
		return true;
	}

    string get(string key)
	{
		this->_reply = (redisReply*)redisCommand(this->_connect, "GET %s", key.c_str());
		
		if(this->_reply->type == REDIS_REPLY_NIL)
		{
			printf("Get Data %s failed\n",key.c_str());
			return "";
		}
		string str = this->_reply->str;
		freeReplyObject(this->_reply);
		return str;
	}

	void set(string key, string value)
	{
		redisCommand(this->_connect, "SET %s %s", key.c_str(), value.c_str());
	}

	void zset(string key, char *value, long unsigned int size)
	{ 
		const char *v[4];
		size_t vlen[4];
		v[0] = (char *)"zadd";
		vlen[0] = strlen("zadd");
		v[1] = key.c_str();
		vlen[1] = strlen(key.c_str());
		std::stringstream ss;
		ss << time(0);
		v[2] = ss.str().c_str();
		vlen[2] = ss.str().size();
		v[3] = (const char *)value;
		vlen[3] = size;
		redisCommandArgv(this->_connect, sizeof(v) / sizeof(v[0]), v, vlen);
	}
	
	void expire(string key, string time)
	{
		printf("expire %s %s\n", key.c_str(), time.c_str());
		redisCommand(this->_connect, "expire %s %s", key.c_str(), time.c_str());
	}
	
	void pub(string channel, string message)
	{
		redisCommand(this->_connect, "publish %s %s", channel.c_str(), message.c_str());
	}
	
	void del(string key)
	{
		redisCommand(this->_connect, "del %s", key.c_str());
	}
	
	int getllen(string key)
	{
		this->_reply = (redisReply*)redisCommand(this->_connect, "llen %s", key.c_str());
		int str = this->_reply->integer;
		freeReplyObject(this->_reply);
		return str;
	}
	
	string getlindex(string key, int index )
	{
		this->_reply = (redisReply*)redisCommand(this->_connect, "lindex %s %d", key.c_str(),index);
		string str = this->_reply->str;
		freeReplyObject(this->_reply);
		return str;
	}
	void lpush(string key,string value)
	{
		redisCommand(this->_connect, "lpush %s %s", key.c_str(), value.c_str());
	}
	
	string rpop(string key)
	{
		this->_reply = (redisReply*)redisCommand(this->_connect, "rpop %s", key.c_str());
		if(this->_reply->type == REDIS_REPLY_NIL)
			return "";
		string str = this->_reply->str;
		freeReplyObject(this->_reply);
		return str;
	}
	
	string brpop(string key)
	{
		this->_reply = (redisReply*)redisCommand(this->_connect, "brpop %s 0", key.c_str());
		string str = this->_reply->element[1]->str;
		freeReplyObject(this->_reply);
		return str;
	}
	
	
	private:

    	redisContext* _connect;
		redisReply* _reply;
				
};

void GetList(char* argv, char* result[], char* flag, int& count)
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


int main( int nArgc, char ** papszArgv )
{
	typedef bgm::d2::point_xy<double> point;
	typedef bgm::box<point> box;
	typedef bgm::segment<point> segment;
	
	typedef bgi::quadratic<MAX_NODE_SIZE> params;
    typedef bgi::indexable<point> indexable_point;
    typedef bgi::equal_to<point> equal_to_point;
    typedef bi::allocator<point, bi::managed_mapped_file::segment_manager> allocator_point;
    typedef bgi::rtree<point, params, indexable_point, equal_to_point, allocator_point> rtree_point;
	
	typedef bgi::indexable<segment> indexable_segment;
    typedef bgi::equal_to<segment> equal_to_segment;
    typedef bi::allocator<segment, bi::managed_mapped_file::segment_manager> allocator_segment;
    typedef bgi::rtree<segment, params, indexable_segment, equal_to_segment, allocator_segment> rtree_segment;
	
	int myId, numProcs;
	MPI_Init(&nArgc,&papszArgv);
	MPI_Comm_rank(MPI_COMM_WORLD,&myId);
	MPI_Comm_size(MPI_COMM_WORLD,&numProcs);
	double t1,t2;
	
	
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	CPLSetConfigOption("SHAPE_ENCODING", "UTF-8");
	
	Redis *redis = new Redis();
	if(!redis->connect("127.0.0.1", 6379))
	{
		printf("connect redis error!\n");
		exit(0);
		MPI_Finalize();
	}
	
	png_bytep * row_pointers=(png_bytep*)malloc(256*sizeof(png_bytep));
	for(int i = 0; i < 256; i++)
		row_pointers[i] = (png_bytep)malloc(1024);
	
	
	char* task= new char[256];
	char *tmp=new char[256];
	char* tile_params[MAX_TILE_PARAMS];
	bool save_to_file=false;
	int count=0;
	
	rtree_segment * rtree_segment_ptr;
	rtree_point * rtree_point_ptr;
	int x,y,z;
	double r,rr;
	
	char* png_path= new char[256];

	if(myId==0)
    	printf("Service Start. cores:%d\n",numProcs);
    while(1){
		
		sprintf(task,"%s",redis->brpop("tasklist").c_str());
		try{
			if (strlen(task)>0)
			{
				t1=MPI_Wtime();
				GetList(task, tile_params, (char*)"/", count);
			    sprintf(tmp,"./indexes/%s",tile_params[0]);
			    
			    bi::managed_mapped_file file(bi::open_only, tmp);
			    if (tile_params[0][0]=='l')
					rtree_segment_ptr = file.find<rtree_segment>("rtree").first;
				else
					rtree_point_ptr = file.find<rtree_point>("rtree").first;
				r=atof(tile_params[1]);
				z=atoi(tile_params[3]);
				x=atoi(tile_params[4]);
				y=atoi(tile_params[5]);
				rr=0.707*r;
                                int fcount=0;

				sprintf(png_path,"%s-%f-%d-%d-%d",tile_params[0],r,z,x,y);
				 
				png_structp png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
				png_infop info_ptr = png_create_info_struct(png_ptr);	
				FILE *fp = fopen(png_path, "wb+");
				png_init_io(png_ptr, fp);
				png_set_IHDR(png_ptr, info_ptr, 256, 256, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
				png_write_info(png_ptr, info_ptr);
				
				#pragma omp parallel for num_threads(2) 
				for(int i = 0; i < 256; i++)
				{
					for(int j = 0; j < 256; j ++)
					{
						if (tile_params[0][0]=='l')
						{
							row_pointers[i][4*j]  = 255;// red
							row_pointers[i][4*j+1] = 0;// green
							row_pointers[i][4*j+2] = 0;// blue
					    }
					    else
					    {
							row_pointers[i][4*j]  = 255;// red
							row_pointers[i][4*j+1] = 0;// green
							row_pointers[i][4*j+2] = 255;// blue
						}
						row_pointers[i][4*j+3] = 0; // alpha
			
						double web_mercartor_x = ((256*x+j+0.5)/(128<<z)-1)* L;
						double web_mercartor_y = (1-(256*y+i-0.5)/(128<<z))* L;
						
						box pointBuffer(point(web_mercartor_x-r,web_mercartor_y-r),point(web_mercartor_x+r,web_mercartor_y+r));
						box innerpointBuffer(point(web_mercartor_x-rr,web_mercartor_y-rr),point(web_mercartor_x+rr,web_mercartor_y+rr));
							
						if (tile_params[0][0]=='l')
						{
							std::vector<segment> segment_result1;
								
							rtree_segment_ptr->query(bgi::intersects(innerpointBuffer)&&bgi::nearest(point(web_mercartor_x,web_mercartor_y),1),
							 std::back_inserter(segment_result1));
                                                        fcount=segment_result1.size();
							if(fcount>0)
								row_pointers[i][4*j+3] = 155;
							else{
								std::vector<segment> segment_result;
								rtree_segment_ptr->query(bgi::intersects(pointBuffer)&&bgi::nearest(point(web_mercartor_x,web_mercartor_y),1),std::back_inserter(segment_result));
								if(segment_result.size()>0)
								{
									double disttmp = bg::distance(point(web_mercartor_x,web_mercartor_y),segment_result.front());
									if(disttmp<r)
										row_pointers[i][4*j+3] = 155;
								}
							}
						}else
						{
                                                        std::vector<point> point_result1;
							rtree_point_ptr->query(bgi::intersects(innerpointBuffer)&&bgi::nearest(point(web_mercartor_x,web_mercartor_y),1),
							 std::back_inserter(point_result1));
							fcount=point_result1.size();
							if(fcount>0)
		              			row_pointers[i][4*j+3] = 155;
							else{
                                std::vector<point> point_result;
								rtree_point_ptr->query(bgi::intersects(pointBuffer)&&bgi::nearest(point(web_mercartor_x,web_mercartor_y),1),
								 std::back_inserter(point_result));
								if(point_result.size()>0)
								{
									double disttmp = bg::distance(point(web_mercartor_x,web_mercartor_y),point_result.front());
									if(disttmp<r)
                                        row_pointers[i][4*j+3] = 155;
								}	
							}
						}
					}
				}
                
				png_write_image(png_ptr, row_pointers);
				png_write_end(png_ptr, NULL);
				
				fseek(fp ,0,SEEK_END);
				long size=ftell(fp);
				//~ printf("size  :%ld\n",size);
				rewind(fp);
				char *pos=(char*)malloc(size);
				fread(pos, 1, size,fp);
				fclose(fp);
				if(not save_to_file)
					remove(png_path);
				
				redis->zset(task,pos,size);
				redis->expire(task,"1000");
			    redis->pub("newtiles",task);
				
				t2 = MPI_Wtime();
				printf("tile-%s-%d-%f\n",png_path, myId, t2-t1);

			}
		}
		catch(...)
		{
			printf("Error task %s \n",task);
				
		}
	
	}
	/* cleanup heap allocation */
	for (int j=0; j<256; j++)
		free(row_pointers[j]);
	free(row_pointers);
	
	MPI_Finalize();
}
