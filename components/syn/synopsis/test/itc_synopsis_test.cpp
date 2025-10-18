// #include "itc_synopsis_bridge.h"

// //update_priority_bin
// //list_data_product_ids
// //get_data_product

// //msg.get functions?

// typedef enum {
//     E_SUCCESS = 0,
//     E_FAILURE = 1,
//     E_TIMEOUT = 2
//  }ITC_STATUS_MESSAGE;

// class TestASDS : public Synopsis::ASDS {

//     public:

//         int invocations;

//         Synopsis::Status init(size_t bytes, void* memory, Synopsis::Logger *logger) override {
//             this->_logger = logger;
//             this->invocations = 0;
//             return Synopsis::SUCCESS;
//         }

//         Synopsis::Status deinit() override {
//             this->invocations = 0;
//             return Synopsis::SUCCESS;
//         }

//         Synopsis::Status process_data_product(Synopsis::DpMsg msg) override {
//             this->invocations++;
//             return Synopsis::SUCCESS;
//         }

//         size_t memory_requirement(void) override {
//             return 123;
//         }
// };

// static TestASDS itc_asds;
// Synopsis::PassthroughASDS pt_asds;
// Synopsis::SqliteASDPDB db(":memory:");
// Synopsis::StdLogger logger;
// Synopsis::LinuxClock clock2;
// Synopsis::MaxMarginalRelevanceDownlinkPlanner planner;
// Synopsis::Application app(&db, &planner, &logger, &clock2);
// void* memory = malloc(128);

// int itc_setup_testasds(){
//     Synopsis::Status status;
//     status = app.add_asds("test_instrument", &itc_asds);
//     EXPECT_EQ(Synopsis::Status::SUCCESS, status);
//     return status;
// }

// void itc_setup_ptasds(){
//     Synopsis::Status status;
//     status = app.add_asds("test_instrument", "type", &pt_asds);
//     EXPECT_EQ(Synopsis::Status::SUCCESS, status);
// }

// size_t itc_app_get_memory_requiremennt(){
//     size_t mem_req_bytes = 0;
//     mem_req_bytes = app.memory_requirement();        
//     printf("REQ Bytes: %d\n", mem_req_bytes);
//     return mem_req_bytes;
// }

// void itc_app_init(size_t bytes, void* memory){ // Remember to free memory
//     Synopsis::Status status;
//     status = app.init(bytes, memory);
//     EXPECT_EQ(Synopsis::Status::SUCCESS, status);
// }

// void itc_app_deinit(){
//     Synopsis::Status status;
//     status = app.deinit();
//     EXPECT_EQ(Synopsis::Status::SUCCESS, status);
// }

// int itc_app_get_invocations(){
//     int invocations = itc_asds.invocations;
//     printf("INVOCATIONS: %d\n", invocations);
//     return invocations;
// }

// struct itc_dpmsg {
//     Synopsis::DpMsg *obj;
// };

// itc_dpmsg_t* itc_create_dpmsg(char* instrument_name, char* dp_type, char* dp_uri, char* meta_uri, bool meta_usage){
//     itc_dpmsg_t *msg;
//     Synopsis::DpMsg *dpobj;

//     msg = (typeof(msg))malloc(sizeof(*msg));
//     std::string inst_name(instrument_name);
//     std::string type(dp_type);
//     std::string uri(dp_uri);
//     std::string metauri(meta_uri);

//     dpobj = new Synopsis::DpMsg(inst_name, type, uri, metauri, meta_usage);

//     msg->obj = dpobj;

//     return msg;
// }

// struct itc_dbdpmsg{
//     Synopsis::DpDbMsg *obj;
// };

// struct itc_dpmetavalue{
//     Synopsis::DpMetadataValue *obj;
// };

// itc_dpmetavalue_t* itc_create_dpmetadatavalue_int(int int_value){
//     itc_dpmetavalue_t *value;
//     Synopsis::DpMetadataValue *obj;

//     value = (typeof(value))malloc(sizeof(*value));
//     obj = new Synopsis::DpMetadataValue(int_value);

//     value->obj = obj;
//     return value;
// }

// itc_dpmetavalue_t* itc_create_dpmetadatavalue_float(double float_value){
//     itc_dpmetavalue_t *value;
//     Synopsis::DpMetadataValue *obj;

//     value = (typeof(value))malloc(sizeof(*value));
//     obj = new Synopsis::DpMetadataValue(float_value);

//     value->obj = obj;
//     return value;
// }

// itc_dpmetavalue_t* itc_create_dpmetadatavalue_string(char* string_value){
//     itc_dpmetavalue_t *value;
//     Synopsis::DpMetadataValue *obj;

//     value = (typeof(value))malloc(sizeof(*value));
//     std::string s_value(string_value);
//     obj = new Synopsis::DpMetadataValue(s_value);

//     value->obj = obj;
//     return value;
// }

// void itc_map_put(void* map, char* dp_string, itc_dpmetavalue_t* dpmetavalue){
//     std::string dpstring(dp_string);
//     Map* m = reinterpret_cast<Map *>(map);
//     Synopsis::MetadataType value_type = dpmetavalue->obj->get_type();
//     Synopsis::DpMetadataValue realvalue;
//     switch(value_type){
//         case 0: //int
//             realvalue = Synopsis::DpMetadataValue(dpmetavalue->obj->get_int_value());
//             break;
//         case 1: //float
//             realvalue = Synopsis::DpMetadataValue(dpmetavalue->obj->get_float_value());
//             break;
//         case 2: //string
//             realvalue = Synopsis::DpMetadataValue(dpmetavalue->obj->get_string_value());
//             break;
//         default:
//             printf("Invalid value Type when creating DpMetaDataValue!\n");
//     }
//     m->insert(std::pair<std::string, Synopsis::DpMetadataValue>(dpstring, realvalue));
// }

// struct itc_node {
//     char* value_type;
//     itc_dpmetavalue_t *meta_data_value;
//     itc_node_t *next = NULL;

// };

// void itc_node_push(itc_node_t* head, char* word, itc_dpmetavalue_t* metavalue){
//     itc_node_t* current = head;
//     if(current != NULL){
//         while(current->next != NULL){
//             current = current->next;
//         }
//     }
//     current = (itc_node_t*) malloc(sizeof(itc_node_t));
//     current->value_type = (char *)malloc(strlen(word));
//     strcpy(current->value_type, word);
//     current->meta_data_value = metavalue;
//     //printf("Word to be inserted %s\n", word);
//     printf("Word Inserted: %s\n", (char *)current->value_type);
//     current->next = NULL;
// }

// itc_dbdpmsg_t* itc_create_dbdpmsg(int dp_id, const char* instrument_name, const char* dp_type, const char* dp_uri, size_t dp_size, double science_utility_estimate, int priority_bin, int downlink_state, itc_node_t* meta_node){
//     itc_dbdpmsg_t *msg;
//     Synopsis::DpDbMsg *dbdpobj;
//     std::map<std::string, Synopsis::DpMetadataValue> mp;
//     itc_node_t *current = meta_node;
//     while(current != NULL){
//         Synopsis::DpMetadataValue *tempdpvalue = static_cast<Synopsis::DpMetadataValue *>(current->meta_data_value->obj);
//         Synopsis::MetadataType value_type = tempdpvalue->get_type();
//         Synopsis::DpMetadataValue realvalue;
//         switch(value_type){
//         case 0: //int
//             realvalue = Synopsis::DpMetadataValue(tempdpvalue->get_int_value());
//             break;
//         case 1: //float
//             realvalue = Synopsis::DpMetadataValue(tempdpvalue->get_float_value());
//             break;
//         case 2: //string
//             realvalue = Synopsis::DpMetadataValue(tempdpvalue->get_string_value());
//             break;
//         default:
//             printf("Invalid value Type when creating DpMetaDataValue!\n");
//         }
//         mp.insert({std::string(current->value_type), realvalue});
//     }

//     msg = (typeof(msg))malloc(sizeof(*msg));
//     Synopsis::DownlinkState dl_state;
//     switch(downlink_state){
//         case 0:
//             dl_state = Synopsis::DownlinkState::UNTRANSMITTED;
//             break;
//         case 1:
//             dl_state = Synopsis::DownlinkState::TRANSMITTED;
//             break;
//         case 2: 
//             dl_state = Synopsis::DownlinkState::DOWNLINKED;
//             break;
//         default:
//             printf("This is an invalid downlink state must be 0-2!\n");
//     }
//     printf("Tis is a test: %s\n", instrument_name);
//     std::string inst_name(instrument_name);
//     std::string dptype(dp_type);
//     std::string dpuri(dp_uri);
//     dbdpobj = new Synopsis::DpDbMsg(dp_id, inst_name, dptype, dpuri, dp_size, science_utility_estimate, priority_bin, dl_state, (Synopsis::AsdpEntry)mp);
   
//     msg->obj = dbdpobj;
    
//     return msg;
// }

// void itc_db_init(size_t bytes, void* memory){
//     Synopsis::Status status;
//     status = db.init(bytes, memory, &logger);
//     EXPECT_EQ(Synopsis::Status::SUCCESS, status);
// }

// void itc_db_insert_dumb_data_product(){
//     Synopsis::Status status;
//     Synopsis::DpDbMsg msg(
//         -1, "test_instr", "test_type", "file:///data/file.dat",
//         101, 0.12345, 7, Synopsis::DownlinkState::UNTRANSMITTED,
//         {
//             {"test_int", Synopsis::DpMetadataValue(123)},
//             {"test_float", Synopsis::DpMetadataValue(123.456)},
//             {"test_string", Synopsis::DpMetadataValue("test")}
//         }
//     );
//     status = db.insert_data_product(msg);
//     EXPECT_EQ(Synopsis::Status::SUCCESS, status);
// }

// void itc_app_accept_dumb_dpmsg(){
//     Synopsis::Status status;
//     Synopsis::DpMsg msg(
//         "test_instrument", "test_type",
//         "file::///data/file.dat",
//         "file::///data/meta.dat",
//         true
//     );

//     status = app.accept_dp(msg);
//     EXPECT_EQ(Synopsis::Status::SUCCESS, status);
// }

// void itc_app_accept_dpmsg(itc_dpmsg_t* msg){
//     Synopsis::Status status;
//     status = app.accept_dp(*(msg->obj));
//     EXPECT_EQ(Synopsis::Status::SUCCESS, status);
// }

// void itc_db_insert_data_product(itc_dbdpmsg_t *msg){
//     Synopsis::Status status;
//     status = db.insert_data_product(*(msg->obj));
//     EXPECT_EQ(Synopsis::Status::SUCCESS, status);
// }

// int itc_msg_get_dp_id(itc_dbdpmsg_t * msg){
//     Synopsis::DpDbMsg *tempdbmsg = static_cast<Synopsis::DpDbMsg *>(msg->obj);
//     return tempdbmsg->get_dp_id();
// }

// const char* itc_msg_get_instrument_name(itc_dbdpmsg_t* msg){
//     Synopsis::DpDbMsg *tempdbmsg = static_cast<Synopsis::DpDbMsg *>(msg->obj);
//     std::string str = (tempdbmsg->get_instrument_name());
//     printf("RB: %s\n", str.c_str());
//     const char* retval = str.c_str();
//     return retval;
// }

// struct itc_dpids{
//     int* values;
//     int size;
// };

// //FREE ME
// itc_dpids_t *itc_db_list_data_product_ids(){
//     std::vector<int> asdp_ids = db.list_data_product_ids();
//     itc_dpids_t *array = (itc_dpids_t *)malloc(sizeof(itc_dpids_t));
//     int n = asdp_ids.size();
//     array->size = n;
//     array->values = (int *)malloc(n * sizeof(int));
//     std::copy(asdp_ids.begin(), asdp_ids.end(), array->values);
//     return array;
// }

// //FREE ME
// itc_dbdpmsg_t* itc_db_get_data_product(int id, itc_dbdpmsg_t *msg){
//     Synopsis::Status status;
//     //msg = (typeof(msg))malloc(sizeof(*msg));
//     Synopsis::DpDbMsg temp_msg2;
//     status = db.get_data_product(id, temp_msg2);

//     printf("MESSAGE2: %s\n", temp_msg2.get_instrument_name().c_str());
//     msg->obj = &temp_msg2;   

//     EXPECT_EQ(Synopsis::Status::SUCCESS, status);  

//     return msg;  
// }

// //TODO:  ASK:  What is the purpose of this DP_ID?  How is it utilized.


// //TODO:  Find any memory that needs to be freed.

// TEST(SynopsisTest, TestCWrapper){
//     size_t mem_req_bytes = 0;

//     itc_setup_testasds();
//     itc_setup_ptasds();
//     mem_req_bytes = itc_app_get_memory_requiremennt();

//     itc_app_init(mem_req_bytes, memory);
//     EXPECT_EQ(0, itc_app_get_invocations());

//     itc_app_accept_dumb_dpmsg();
//     EXPECT_EQ(1, itc_app_get_invocations());
    
//     itc_app_accept_dumb_dpmsg();
//     EXPECT_EQ(2, itc_app_get_invocations());

//     itc_dpmsg_t *temp_dpmsg = itc_create_dpmsg((char *)"test_instrument", (char *)"test_type", (char *)"file::///data/file.dat", (char *)"file::///data/meta.dat", true);
//     itc_app_accept_dpmsg(temp_dpmsg);
//     EXPECT_EQ(3, itc_app_get_invocations());

//     itc_node_t *start = NULL;
//     itc_node_push(start, (char *)"test_int", itc_create_dpmetadatavalue_int(123));
//     itc_node_push(start, (char *)"test_float", itc_create_dpmetadatavalue_float(123.456));
//     itc_node_push(start, (char *)"test_string", itc_create_dpmetadatavalue_string((char *)"test"));

//     itc_db_init(0, NULL);

//     itc_dbdpmsg_t* dbmsg = itc_create_dbdpmsg(-1, "test_instr", "test_type", "file:///data/file.dat", 101, 0.12345, 7, 0, start);

//     itc_db_insert_data_product(dbmsg);
//     EXPECT_GT(itc_msg_get_dp_id(dbmsg), 0);

//     itc_dpids_t* asdp_ids = itc_db_list_data_product_ids();
//     EXPECT_EQ(asdp_ids->size, 1);

//     itc_dbdpmsg_t *copy_msg = (typeof(copy_msg))malloc(sizeof(*copy_msg));
//     itc_db_get_data_product(1, copy_msg);

//     EXPECT_EQ(itc_msg_get_dp_id(dbmsg), itc_msg_get_dp_id(copy_msg));
//     //printf("MESSAGE: %s\n", itc_msg_get_instrument_name(dbmsg));
//     //printf("MESSAGE: %s\n", itc_msg_get_instrument_name(copy_msg));
//     // EXPECT_EQ(itc_msg_get_instrument_name(dbmsg), itc_msg_get_instrument_name(copy_msg));
//     // printf("DEBUGGING MSG: %s", dbmsg->obj->get_instrument_name());
//     // printf("DEBUGGING MSG: %s", dbmsg->obj->get_type());
//     // printf("DEBUGGING MSG: %s", dbmsg->obj->get_uri());
//     // printf("DEBUGGING MSG: %ld", dbmsg->obj->get_dp_size());
//     // printf("DEBUGGING MSG: %f", dbmsg->obj->get_science_utility_estimate());
//     // printf("DEBUGGING MSG: %d", dbmsg->obj->get_priority_bin());
//     // printf("DEBUGGING MSG: %d", dbmsg->obj->get_downlink_state());
//     // printf("DEBUGGING MSG: %d", dbmsg->obj->get_dp_id());


//     //EXPECT_EQ(dbmsg->obj->get_instrument_name(), copy_msg->obj->get_instrument_name());
//     // EXPECT_EQ(dbmsg->obj->get_type(), copy_msg->obj->get_type());
//     // EXPECT_EQ(dbmsg->obj->get_uri(), copy_msg->obj->get_uri());
//     // EXPECT_EQ(dbmsg->obj->get_dp_size(), copy_msg->obj->get_dp_size());
//     // EXPECT_EQ(dbmsg->obj->get_science_utility_estimate(), copy_msg->obj->get_science_utility_estimate());
//     // EXPECT_EQ(dbmsg->obj->get_priority_bin(), copy_msg->obj->get_priority_bin());
//     // EXPECT_EQ(dbmsg->obj->get_downlink_state(), copy_msg->obj->get_downlink_state());
    
//     itc_app_deinit();
//     EXPECT_EQ(0, itc_app_get_invocations());
// }
