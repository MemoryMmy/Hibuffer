redis-cli flushall
ps -ef | grep ./Buffer | grep -v grep| awk '{print "kill -9 " $2}'| sh
ps -ef | grep ./uwsgitileserver/server.py | grep -v grep| awk '{print "kill -9 " $2}'| sh
