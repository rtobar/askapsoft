--- src/Outputs/CatalogueSpecification.hh.orig	2014-05-02 10:02:03.000000000 +1000
+++ src/Outputs/CatalogueSpecification.hh	2015-01-09 11:11:51.000000000 +1100
@@ -47,6 +47,7 @@
       virtual ~CatalogueSpecification(){};
 
       void addColumn(Column col);
+      void addColumn(std::string type, std::string name, std::string units, int width, int prec, std::string ucd="", std::string datatype="", std::string colID="", std::string extraInfo="");
       Column &column(std::string type){return itsColumnList[itsTypeMap[type]];};
       Column &column(int i){return itsColumnList[i];};
       Column *pCol(int i){return &(itsColumnList[i]);};
