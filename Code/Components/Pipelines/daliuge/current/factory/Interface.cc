// expose the member functions to the outside world as C functions
// If Daliuge changes its API - you will have to change these functions
// but hopefully all the issues will be hidden behind the structures

#include "askap_daliuge_pipeline.h"

#include<daliuge/DaliugeApplication.h>
#include<factory/DaliugeApplicationFactory.h>
#include<factory/Interface.h>


    int init(dlg_app_info *app, const char ***arguments) {
        // this means we have to instantiate an application
        // and call its init
        askap::DaliugeApplication::ShPtr thisApp = askap::DaliugeApplicationFactory::make(app->appname);
        return thisApp->init(app, arguments);
    }
    int run(dlg_app_info *app) {
        askap::DaliugeApplication::ShPtr thisApp = askap::DaliugeApplicationFactory::make(app->appname);
        return thisApp->run(app);
    }
    void data_written(dlg_app_info *app, const char *uid,
        const char *data, size_t n) {
            askap::DaliugeApplication::ShPtr thisApp = askap::DaliugeApplicationFactory::make(app->appname);
            thisApp->data_written(app, uid, data, n);
    }
    void drop_completed(dlg_app_info *app, const char *uid,
        drop_status status) {
            askap::DaliugeApplication::ShPtr thisApp = askap::DaliugeApplicationFactory::make(app->appname);
            thisApp->drop_completed(app, uid, status);
    }
