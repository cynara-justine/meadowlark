/*
 *  (c) Copyright 2016-2017, 2021 Hewlett Packard Enterprise Development Company LP.
 *
 *  This software is available to you under a choice of one of two
 *  licenses. You may choose to be licensed under the terms of the
 *  GNU Lesser General Public License Version 3, or (at your option)
 *  later with exceptions included below, or under the terms of the
 *  MIT license (Expat) available in COPYING file in the source tree.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  As an exception, the copyright holders of this Library grant you permission
 *  to (i) compile an Application with the Library, and (ii) distribute the
 *  Application containing code generated by the Library and added to the
 *  Application during this compilation process under terms of your choice,
 *  provided you also meet the terms and conditions of the Application license.
 *
 */

#include "kvs_client/kvs_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <cstddef> // size_t
#include <unordered_map>
#include <functional> // hash

#include "libmemcached/memcached.h"
#include "cluster/config.h"
#include "cluster/cluster.h"

namespace radixtree {

KVSServer::KVSServer() {
}

KVSServer::~KVSServer() {
}

int KVSServer::Init (std::string config_file) {
    int ret = config_.LoadConfigFile(config_file);
    if(ret==0) {
        assert(config_.IsValid());
        cluster_.Init(config_);
    }
    return ret;
}

void KVSServer::Final() {
    cluster_.Final();
    for(auto i:servers_) {
        memcached_free(i.second);
    }
}

int KVSServer::Put (char const *key, size_t const key_len,
                    char const *val, size_t const val_len) {
    bool failed_once=false;
    int ret=0;
    do {
        memcached_st **server = get_server(key, key_len);
        //assert(server);
        ret=0;
        memcached_return_t rc = memcached_set(*server, key, key_len, val, val_len, (time_t)0, (uint32_t)0);
        if(rc!=MEMCACHED_SUCCESS) {
            std::cout << "KVSServer put error: " << memcached_strerror(*server, rc) << std::endl;
            *server=NULL;
            if(failed_once) return -1;
            else failed_once=true;
            ret=-1;
        }
#ifdef DEBUG
        else {
            std::cout << "KVSServer put: " << std::string(key, key_len) << " -> " << std::string(val, val_len) << std::endl;
        }
#endif
    }
    while(ret==-1);
    return ret;
}

int KVSServer::Get (char const *key, size_t const key_len,
                    char *val, size_t &val_len){
    bool failed_once=false;
    int ret=0;
    do {
        memcached_st **server = get_server(key, key_len);
        //assert(server);
        ret=0;
        memcached_return_t rc;
        uint32_t flags;
        char *res = memcached_get(*server, key, key_len, &val_len, &flags, &rc);
        if(!res) {
            std::cout << "KVSServer get error: " << memcached_strerror(*server, rc) << std::endl;
            *server=NULL;
            if(failed_once) return -1;
            else failed_once=true;
            ret=-1;
        }
#ifdef DEBUG
        else {
            std::cout << "KVSServer get: " << std::string(key, key_len) << " -> " << std::string(res, val_len) << std::endl;
        }
#endif
        memcpy(val, res, val_len);
        free(res);
    }
    while(ret==-1);
    return ret;
}

int KVSServer::Del (char const *key, size_t const key_len) {
    bool failed_once=false;
    int ret=0;
    do {
        memcached_st **server = get_server(key, key_len);
        //assert(server);
        ret=0;
        memcached_return_t rc = memcached_delete(*server, key, key_len, (time_t)0);
        if(rc!=MEMCACHED_SUCCESS) {
            std::cout << "KVSServer del error: " << memcached_strerror(*server, rc) << std::endl;
            *server=NULL;
            if(failed_once) return -1;
            else failed_once=true;
            ret=-1;
        }
#ifdef DEBUG
        else {
            std::cout << "KVSServer del: " << std::string(key, key_len) << std::endl;
        }
#endif
    }
    while(ret==-1);
    return ret;
}

void KVSServer::PrintCluster() {
    cluster_.Print();
}

void KVSServer::WipeServers() {
    for(auto i:servers_) {
        int ret=memcached_flush(i.second, (time_t)0);
        assert(ret==MEMCACHED_SUCCESS);
    }
}

Location KVSServer::pick_a_server(char const *key, size_t const key_len) {
    ResourceName r(std::string(key, key_len));
    return cluster_.Locate(r, true);
}

int KVSServer::server_init(Location loc) {
    int ret=0;
    std::string dest = "--SERVER="+loc.ip+":"+std::to_string(loc.port);
    memcached_st *server = memcached(dest.c_str(), dest.size());
    if(!server) {
        std::cout << "memcached server init error: " << loc << std::endl;
        ret=-1;
    }
#ifdef DEBUG
    else {
        std::cout << "memcached server connected: " << loc << std::endl;
    }
#endif
    assert(servers_.find(loc)==servers_.end());
    servers_[loc]=server;
    return ret;
}

bool KVSServer::server_exist(Location loc) {
    return servers_.find(loc)!=servers_.end();
}

memcached_st **KVSServer::get_server(char const *key, size_t const key_len) {
    Location loc;
    int cnt=0;
    do {
        loc = pick_a_server(key, key_len);
        if(!server_exist(loc)) {
            int ret = server_init(loc);
            if(ret!=0)
                return nullptr;
        }
        if(servers_[loc]==NULL) {
            // TODO: fix this
            cnt++;
            if(cnt>=100)
                return nullptr;
        }
        else {
            break;
        }
    }
    while(1);
            
    //std::cout << "Using server " << loc << std::endl;
    return &servers_[loc];
}

}
