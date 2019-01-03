# HiBuffer
Buffer analysis is a fundamental function in geographic information system (GIS), which identifies areas by
surrounding geographic features in a given distance. Real-time buffer analysis for large scale spatial data remains a challenging problem since the computational scales of conventional data-oriented methods expand rapidly with data volumes. In this program, we present a visualization-oriented model, HiBuffer, for real-time buffer analysis. Experiments on real-world datasets show that our approach can reduce computation time by up to orders of magnitude while preserving better visualization effects. An demonstration of HiBuffer is provided on the Web. The core code of HiBuffer is implemented using the ***C++*** language.



## Online Demo

***Tab1. Datasets of [Demo 1](http://www.higis.org.cn:8080/hibuffer/): Roads and POI of Spain from [OpenStreetMap](https://download.geofabrik.de/europe/spain-latest.osm.pbf)***

| Name           | Type       | Records    | Size                |
| -------------- | ---------- | ---------- | ------------------- |
| OSM_Spain_Road | LineString | 3,132,496  | 42,497,196 segments |
| OSM_Spain_POI  | Point      | 355,105    | 355,105 points      |


***Tab2. Datasets of [Demo 2](http://www.higis.org.cn:8080/hibuffer10million/): Roads, POI and Farmland of Mainland China (10-million-scale)***

| Name           | Type       | Records    | Size                |
| -------------- | ---------- | ---------- | ------------------- |
| China_Road     | LineString | 21,898,508 | 163,171,928 segments|
| China_POI      | Point      | 20,258,450 | 20,258,450 points   |
| China_Farmland | Polygon    | 10,520,644 | 133,830,561 edges   |


***Tab3.  Demo Environment***

| Item             | Description                                      |
| ---------------- | ------------------------------------------------ |
| CPU              | 4core*2, Intel(R) Xeon(R) CPU E5-2680 v3@2.50GHz |
| Memory           | 32 GB                                            |
| Operating System | Centos7                                          |



## Citation

Ma M, Wu Y, et al. HiBuffer: Buffer Analysis of 10-Million-Scale Spatial Data in Real Time[J]. International Journal of Geo-Information, 2018, 7(12):467. DOI: 10.3390/ijgi7120467


## Undergoing Work: 
### [Interactive and Online Buffer and Overlay Analytics（support polygon objects）](https://github.com/MemoryMmy/HiBO) ([Online Demo](http://www.higis.org.cn:8080/hibo/))

## Contact:

Mengyu Ma@ National University of Defense Technology

Email: mamengyu10@nudt.edu.cn

Tel:+8615507487344

To be open source in the future. 
