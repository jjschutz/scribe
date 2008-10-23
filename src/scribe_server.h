//  Copyright (c) 2007-2008 Facebook
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
// See accompanying file LICENSE or visit the Scribe site at:
// http://developers.facebook.com/scribe/ 
//
// @author Bobby Johnson
// @author James Wang
// @author Jason Sobel
// @author Avinash Lakshman
// @author Anthony Giardullo

#ifndef SCRIBE_SERVER_H
#define SCRIBE_SERVER_H

#include "store.h"
#include "store_queue.h"

typedef std::vector<boost::shared_ptr<StoreQueue> > store_list_t;
typedef std::map<std::string, boost::shared_ptr<store_list_t> > category_map_t;
typedef std::map<std::string, boost::shared_ptr<StoreQueue> > category_prefix_map_t;

class scribeHandler : virtual public scribe::thrift::scribeIf, 
                              public facebook::fb303::FacebookBase {

 public:
  scribeHandler(unsigned long int port, const std::string& conf_file);
  ~scribeHandler();

  void shutdown();
  void initialize();
  void reinitialize();

  scribe::thrift::ResultCode Log(const std::vector<scribe::thrift::LogEntry>& messages);

  void getVersion(std::string& _return) {_return = "2.0";}
  facebook::fb303::fb_status getStatus();
  void getStatusDetails(std::string& _return);
  void setStatus(facebook::fb303::fb_status new_status);
  void setStatusDetails(const std::string& new_status_details);

  unsigned long int port; // it's long because that's all I implemented in the conf class

 private:
  unsigned long checkPeriod; // periodic check interval for all contained stores

  // This map has an entry for each configured category.
  // Each of these entries is a map of type->StoreQueue.
  // The StoreQueue contains a store, which could contain additional stores.
  category_map_t* pcategories;
  category_prefix_map_t* pcategory_prefixes;

  // the default store
  boost::shared_ptr<StoreQueue> defaultStore;

  std::string configFilename;
  facebook::fb303::fb_status status;
  std::string statusDetails;
  facebook::thrift::concurrency::Mutex statusLock;
  time_t lastMsgTime;
  unsigned long numMsgLastSecond;
  unsigned long maxMsgPerSecond;
  unsigned long maxQueueSize;
  bool newThreadPerCategory;

  // disallow empty construction, copy, and assignment
  scribeHandler();
  scribeHandler(const scribeHandler& rhs);
  const scribeHandler& operator=(const scribeHandler& rhs);

 protected:
  bool throttleDeny(int num_messages); // returns true if overloaded
  void deleteCategoryMap(category_map_t *pcats);
  const char* statusAsString(facebook::fb303::fb_status new_status);
  bool createCategoryFromModel(const std::string &category, 
                               const boost::shared_ptr<StoreQueue> &model);
};

extern boost::shared_ptr<scribeHandler> g_Handler;

#endif // SCRIBE_SERVER_H