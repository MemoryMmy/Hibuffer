# Hibuffer
Buffer analysis is a fundamental function in geographic information system (GIS), which identifies areas by
surrounding geographic features in a given distance. Real-time buffer analysis for large scale spatial data remains a challenging problem since the computational scales of conventional data-oriented methods expand rapidly with data volumes. In this paper, we introduce HiBuffer, a visualization-oriented model for
real-time buffer analysis. An efficient buffer generation method, which introduces spatial indexes and a corresponding query strategy, is proposed. Buffer results are organized into a tile-pyramid structure to enable stepless zooming. Moreover, a fully optimized hybrid-parallel processing architecture is proposed for real-time buffer analysis of large-scale spatial data. Experiments on real-world datasets show that our approach can reduce computation time by up to orders of magnitude while preserving better visualization effects. An online demonstration of HiBuffer is provided on the Web. [Online demo](http://www.higis.org.cn:8080/hibuffer/)

<<<<<<< HEAD


** Datasets & Experimental Environment of the Demonstration 

***[Tab1. Dataset](https://download.geofabrik.de/europe/spain-latest.osm.pbf) ***
=======
***Tab1. dataset***    [From OpenStreetMap](https://download.geofabrik.de/europe/spain-latest.osm.pbf)
>>>>>>> 8d50fc2e336fd0dc95b9c764f560796563e45a0d

| Name           | Type       | Size                |
| -------------- | ---------- | ------------------- |
| OSM_Spain_Road | LineString | 42,497,196 segments |
| OSM_Spain_POI  | Point      | 355,105 points      |

<<<<<<< HEAD
***Tab2.  Experimental Environment***
=======

***Tab2.  Experimental environment***
>>>>>>> 8d50fc2e336fd0dc95b9c764f560796563e45a0d

| Item             | Description                                                  |
| ---------------- | ------------------------------------------------------------ |
| CPU              | 4core*2, Intel(R) Xeon(R) CPU E5-2680 v3@2.50GHz             |
| Memory           | 32 GB                                                        |
| Software         | [MPICH](http://www.mpich.org/) 3.0.4, [Boost C++ Libraries](https://www.boost.org/) 1.64, [Geospatial Data Abstraction Library (GDAL)](http://www.gdal.org/) 2.1.2 |
| Operating System | Centos7                                                      |

