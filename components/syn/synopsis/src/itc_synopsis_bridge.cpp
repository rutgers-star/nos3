#include "itc_synopsis_bridge.h"

#include "synopsis.hpp"
#include "SqliteASDPDB.hpp"
#include "StdLogger.hpp"
#include "LinuxClock.hpp"
#include "MaxMarginalRelevanceDownlinkPlanner.hpp"
#include "Timer.hpp"
#include "RuleAST.hpp"
#include "StdLogger.hpp"
#include <cstring>
#include <vector>
#include <algorithm>

#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

 typedef enum {
    E_SUCCESS = 0,
    E_FAILURE = 1,
    E_TIMEOUT = 2
 }ITC_STATUS_MESSAGE;

#define SYNOPSIS_DATA_PATH "./data/owls/"
#define SYNOPSIS_DATABASE_PATH "./data/owls/db_new.db"
#define SYNOPSIS_DATA_BUNDLE_PATH "./data/owls/bundle/asdp00000000"
#define SYNOPSIS_SIMILARITY_CONFIG_PATH "./data/owls/owls_similarity_config.json"
#define SYNOPSIS_RULES_PATH "./data/owls/empty_rules.json"

//Synopsis::SqliteASDPDB db(":memory:"); //Can just use in-memory
Synopsis::SqliteASDPDB db(SYNOPSIS_DATABASE_PATH);

Synopsis::StdLogger logger;
Synopsis::LinuxClock clock2;
Synopsis::MaxMarginalRelevanceDownlinkPlanner planner;

Synopsis::Application app(&db, &planner, &logger, &clock2);
Synopsis::PassthroughASDS pt_asds;

std::vector<std::string> prioritized_uris;
std::vector<int> prioritized_list;

int dp_counter = 0;  // This is a dumb counter for this example.  Users need to handle how their data will be managed.

/**
 * ITC Function to reset the value of the DP MSG Counter to 0
*/
void reset_dp_counter(){
    dp_counter = 0;
}

/**
 * ITC Function to get the value of the DP MSG Counter
*/
int get_dp_counter(){
    return dp_counter;
}

std::string get_absolute_data_path(std::string relative_path_str) {
    char sep = '/';
    const char* env_p = std::getenv("SYNOPSIS_TEST_DATA");
    std::string base_path(env_p);
    if (base_path[base_path.length()] != sep) {
        return base_path + sep + relative_path_str;
    } else {
        return base_path + relative_path_str;
    }
}

/**
 * ITC Function for creation of dpmsg based on Owls pre-canned data
 * Note:  This requires some asset directory setup and the existence of data or some erroneous behavior will occur.
 * @return: Synopsis::Status resulting from the app.accept_dp wrapped within
*/

int owls_add_dpmsg(){ 
    Synopsis::Status status = Synopsis::Status::FAILURE;

    if (dp_counter > 7){
        printf("*! Unable to add additional Data!\n");
        status = Synopsis::Status::FAILURE;
    }
    else{
        printf("** SYNOPSIS: Adding Data Product %d\n", dp_counter);
        std::string data_path(SYNOPSIS_DATA_BUNDLE_PATH + std::to_string(dp_counter) + ".tgz");
        std::string metadata_path(SYNOPSIS_DATA_BUNDLE_PATH + std::to_string(dp_counter) + "_meta.json");        

        printf("STRINGS: \n\t%s\n\t%s\n", data_path.c_str(), metadata_path.c_str());
        
        Synopsis::DpMsg msg("owls", "helm", data_path, metadata_path, true);

        status = app.accept_dp(msg);
    }    
    
    if (status != Synopsis::Status::SUCCESS){
        printf("*! Error Adding DPMSG to DB!\n");
    }
    else{
        printf("** Adding DPMSG to DB!\n");
        dp_counter++;
    }

    return status;
}

/**
 * ITC Function to update the Sigma Value
 * Modifies the SImilarity Config
*/
void owls_set_sigma(double sigma)
{
    printf("* INCOMING SIGMA: %f\n", sigma);
    std::string similarity(SYNOPSIS_SIMILARITY_CONFIG_PATH);
    std::ifstream json_in(similarity);
    json j_sigma;
    json_in >> j_sigma;
    j_sigma["alphas"]["default"] = sigma;

    std::ofstream json_out(similarity);
    json_out << std::setw(4) << j_sigma << std::endl; 
}

/**
 * ITC Function for prioritization of pre-canned OWLs data
 * Note:  This requires some setup and the existence of files or some erroneous behavior will occur.
 * @return: Synopsis::Status resulting from the app.prioritize wrapped within
*/
int owls_prioritize_data(){
    if(prioritized_list.size() != 0) {prioritized_list.clear();}
    std::string rule_file(SYNOPSIS_RULES_PATH);
    std::string similarity(SYNOPSIS_SIMILARITY_CONFIG_PATH);
    
    Synopsis::Status status;    
    
    status = app.prioritize(rule_file, similarity, 1e9, prioritized_list);
    if (status != Synopsis::Status::SUCCESS) { return status; }

    if(prioritized_uris.size() != 0) {prioritized_uris.clear();}
    
    for (auto i : prioritized_list) {
        Synopsis::DpDbMsg temp_msg;
        app.get_data_product(i, temp_msg);
        prioritized_uris.push_back(temp_msg.get_uri());
    }
    for (auto uri : prioritized_uris){
        printf("** SYNOPSIS URI: %s\n", uri.c_str());
    }

    return Synopsis::Status::SUCCESS;
}

/**
 * ITC Function displaying current prioritized data
 * Note:  This requires some setup and the existence of data or some erroneous behavior will occur.
 * @return: Synopsis::Status::SUCCESS
*/
int owls_display_prioritized_data(){
    for (auto uri: prioritized_uris){
        printf("URI: %s\n", uri.c_str());
    }
    return Synopsis::Status::SUCCESS;
}

/**
 * ITC Function for retreiving the asdp_id from global lists
 * @param: char* uri: The URI of the DpDbMsg from which to retreive the asdp_id
 * @return: int: asdp_id
*/
int get_aspd_id(char* uri){
    int returnval = -1;
    
    std::string orig(uri);
    for (auto i : prioritized_list){
        Synopsis::DpDbMsg temp_msg;
        app.get_data_product(i, temp_msg);
        std::string comp(temp_msg.get_uri().c_str());
        int match = orig.compare(temp_msg.get_uri());
        if( match == 0){
            returnval = temp_msg.get_dp_id();
            break;
        }
    }
    return returnval;
}

/**
 * ITC Function for the simulation of dowlinking prioritized files
 * Note:  This function retreives the highest priority file, and returns its name.uses cFS OS calls
 * to OS_mv the data to a simulated downlink location.  It then updates the downlink state
 * within the DB so that it is no longer part of the prioritization list.
 * @return: char*: The URI of the prioritized data to downlink
*/
char* owls_get_prioritized_data(int index){
    size_t uri_size = prioritized_uris.size();
    if((uri_size != 0) && (0 <= index <= 8) && (index < uri_size) ){ // 8 in this case as example data only contains 7 data products
        size_t len = strlen(prioritized_uris[index].c_str())+1;
        char* returnval = new char[len];
        strcpy(returnval, (char*)prioritized_uris[index].c_str());
        
        return returnval;
        }
    else{
        return NULL;
    }
}


/**
 * ITC Function to convert the most prioritized file status to DONWLINKED
*/
void owls_update_downlink_status_prio1(){
    size_t uri_size = prioritized_uris.size();
    if(uri_size !=0) //change to list 
    {
        printf("prioritized list\n");
        for(int i: prioritized_list)
            std::cout << i << ' ';
        
        int aspd_id = prioritized_list[0];
        db.update_downlink_state(aspd_id, Synopsis::DownlinkState::DOWNLINKED);

        // //char* dpmsg_name = (char *)prioritized_uris[0].c_str();
        // owls_update_downlink_status(dpmsg_name);
        printf("** SYNOPSIS DP Updated\n");
        owls_prioritize_data();
        printf("** SYNOPSIS Priotization Complete\n");
    }
}

/**
 * ITC Function to update dataproduct status on successful downlink
 * Modifies the DLState to DOWNLINKED
*/
void owls_update_downlink_status(char* dpname){
    auto itr = std::find(prioritized_uris.begin(), prioritized_uris.end(), dpname);
    if(itr != prioritized_uris.end()){
        int aspd_id = get_aspd_id(dpname);
        Synopsis::Status status = db.update_downlink_state(aspd_id, Synopsis::DownlinkState::DOWNLINKED);
        printf("** SYNOPSIS DP UPDATED\n");
    }   
    else{
        printf("*! SYNOPSIS DP NOT FOUND!\n");
    }
}        

/**
 * ITC Function for the destruction of data string
 * Deletes the char array created by owls_get_prioritized_data
*/
void owls_destroy_prioritized_data_string(char* deleteme){
    delete[] deleteme;
}

/**
 * ITC Function for setup of the PassThrough ASDS
 * @return: VOID
*/
void itc_setup_ptasds(){
    Synopsis::Status status;
    status = app.add_asds("owls", "helm", &pt_asds);
    ITC_STATUS_MESSAGE result = (ITC_STATUS_MESSAGE)status;
    if(result == E_SUCCESS){
        printf("** SYNOPSIS PTASDS SETUP SUCCESSFUL!\n");
    }
    else{
        printf("*! SYNOPSIS PTASDS SETUP UNSUCCESSFUL!\n");
    }
}

/**
 * ITC Function for setup of the TEST ASDS Class
 * Note:  This requires some setup or some erroneous behavior will occur.
 * @return: size_t memory requirement size required by app
*/
size_t itc_app_get_memory_requiremennt(){
    size_t mem_req_bytes = 0;
    mem_req_bytes = app.memory_requirement();        
    printf("** SYNOPSIS REQ Bytes: %d\n", mem_req_bytes);
    return mem_req_bytes;
}

/**
 * ITC Function for Initializing the App.  
 * Makes use of a memory object that will need to be freed.
 * @return: VOID
*/
void itc_app_init(size_t bytes, void* memory){ 
    Synopsis::Status status;
    
    status = app.init(bytes, memory);
    ITC_STATUS_MESSAGE result = (ITC_STATUS_MESSAGE)status;
    if(result == E_SUCCESS){
        printf("** SYNOPSIS APP INIT SUCCESSFUL!\n");
    }
    else{
        printf("*! SYNOPSIS APP INIT UNSUCCESSFUL!\n");
    }
}

/**
 * ITC Function for De-Initialization of the App
 * @return: VOID
*/
void itc_app_deinit(void* memory){
    Synopsis::Status status;
    status = app.deinit();
    ITC_STATUS_MESSAGE result = (ITC_STATUS_MESSAGE)status;
    if(result == E_SUCCESS){
        printf("** SYNOPSIS DEINIT SUCCESSFUL!\n");
    }
    else{
        printf("*! SYNOPSIS DEINIT UNSUCCESSFUL!\n");
    }
}
