--- src/Outputs/CatalogueSpecification.cc.orig	2014-05-02 10:02:03.000000000 +1000
+++ src/Outputs/CatalogueSpecification.cc	2015-01-09 11:11:51.000000000 +1100
@@ -60,6 +60,13 @@
       this->setMap();
     }
 
+    void CatalogueSpecification::addColumn(std::string type, std::string name, std::string units, int width, int prec, std::string ucd, std::string datatype, std::string colID, std::string extraInfo)
+    {
+        Column col(type,name,units,width,prec,ucd,datatype,colID,extraInfo);
+        this->addColumn(col);
+    }
+      
+      
       void CatalogueSpecification::setMap()
       {
 	  for(size_t i=0;i<this->itsColumnList.size();i++)
