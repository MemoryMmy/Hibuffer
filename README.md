# HiBuffer
Buffer analysis is a fundamental function in geographic information system (GIS), which identifies areas by
surrounding geographic features in a given distance. Real-time buffer analysis for large scale spatial data remains a challenging problem since the computational scales of conventional data-oriented methods expand rapidly with data volumes. In this program, we present a visualization-oriented model, HiBuffer, for real-time buffer analysis. Experiments on real-world datasets show that our approach can reduce computation time by up to orders of magnitude while preserving better visualization effects. An demonstration of HiBuffer is provided on the Web.



## [Online Demo](http://www.higis.org.cn:8080/hibuffer/) 

***Tab1. Datasets: Roads and POI of Spain from [OpenStreetMap](https://download.geofabrik.de/europe/spain-latest.osm.pbf)***

| Name           | Type       | Size                |
| -------------- | ---------- | ------------------- |
| OSM_Spain_Road | LineString | 42,497,196 segments |
| OSM_Spain_POI  | Point      | 355,105 points      |

***Tab2.  Demo Environment***

| Item             | Description                                                  |
| ---------------- | ------------------------------------------------------------ |
| CPU              | 4core*2, Intel(R) Xeon(R) CPU E5-2680 v3@2.50GHz             |
| Memory           | 32 GB                                                        |
| Software         | [MPICH](http://www.mpich.org/) 3.0.4, [Boost C++ Libraries](https://www.boost.org/) 1.64, [Geospatial Data Abstraction Library (GDAL)](http://www.gdal.org/) 2.1.2 |
| Operating System | Centos7                                                      |





## Contact:

Mengyu Ma@ National University of Defense Technology
Email: mamengyu10@nudt.edu.cn



