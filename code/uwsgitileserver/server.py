#    data: 2018-08-10 
#    author: Mengyu Ma@National University of Defense Technology
#    e-mail: mamengyu10@nudt.edu.cn
#    description: Multi-Thread Buffer Tile Server of HiBuffer 
#    run: 
# 	 $ uwsgi --http :55559 --wsgi-file ./uwsgitileserver/server.py --master --processes 16 --threads 2


# -*-coding:utf-8
from cgi import parse_qs
import redis
import logging
import time
import json
import re
from wsgiref.headers import Headers
import httplib
import os
import random
import threading

hostname = "localhost"
r = redis.Redis(host=hostname, port=6379)
L= 20037508.34
def application(environ, start_response):
	
	sub = r.pubsub()
	sub.subscribe("newtiles")
	try:
		ret={}
		path_info = environ.get('PATH_INFO')
		key = path_info.split(".")[0]
		if "OBO" in key:         
			key = key[5:]
			plist=key.split("/")
			z=int(plist[3])
			x=int(plist[4])
			y=int(plist[5])    
			R=float(plist[1])
			dataset=plist[0]
			bbox=r.get(dataset).split(",");
			minx=float(bbox[0])-R;
			miny=float(bbox[1])-R;
			maxx=float(bbox[2])+R;
			maxy=float(bbox[3])+R;
			tile_minx = ((256*x+0.5)/(128<<z)-1)* L;
			tile_miny = (1-(256*y+255.5)/(128<<z))* L;
			tile_maxx = ((256*x+255.5)/(128<<z)-1)* L;
			tile_maxy = (1-(256*y-0.5)/(128<<z))* L;
			print bbox
			if(tile_minx<maxx and tile_miny<maxy and tile_maxx>minx and tile_maxy> miny):
				retContent=r.zrange(key,0,-1)
				if not retContent:
					#~ print "Tile not exists."
					r.lpush("tasklist",key)
					tile=""
					while tile != key:
						tile = sub.parse_response()[2]
					retContent=r.zrange(key,0,-1)
				retContent=retContent[0]
				ret = {'code': '200 OK',
	                       'content': str(retContent),
	                       'headers': [('Content-Type', 'image/png' ), ('Access-Control-Allow-Origin', '*')]}
			else:	
				raise Exception("Tile out range")
	except Exception, e:
		ret = {'code': '500 SERVER ERROR',
			   'content': str(e),
			   'headers': [('Content-Type', 'text/html'),('Content-Length', str(len(str(e))))]}
	finally:
		start_response(ret['code'], ret['headers'])
		return [ret['content']]


