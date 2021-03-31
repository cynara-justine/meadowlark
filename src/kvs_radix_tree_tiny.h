/*
 *  (c) Copyright 2016-2021 Hewlett Packard Enterprise Development Company LP.
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


#ifndef KVS_RADIX_TREE_TINY_H
#define KVS_RADIX_TREE_TINY_H

#include <stddef.h>
#include <stdint.h>
#include <limits>
#include <vector>
#include <mutex>

#include "nvmm/global_ptr.h"
#include "nvmm/shelf_id.h"
#include "nvmm/memory_manager.h"
#include "nvmm/epoch_manager.h"
#include "nvmm/heap.h"

#include "radixtree/kvs.h"
#include "radixtree/radix_tree.h"

#include "kvs_metrics.h"

namespace radixtree {
// TODO: error codes!

using Emgr = nvmm::EpochManager;
using Eop = nvmm::EpochOp;

class KVSRadixTreeTiny : public KeyValueStore {
public:
    static size_t const kMaxKeyLen = RadixTree::MAX_KEY_LEN; // 40 bytes for now
    static size_t const kMaxValLen = 8; // 8 byte

    KVSRadixTreeTiny(Gptr root, std::string base, std::string user, size_t heap_size, nvmm::PoolId heap_id, KVSMetrics* metrics);
    ~KVSRadixTreeTiny();

    void Maintenance();

    int Put (char const *key, size_t const key_len,
	     char const *val, size_t const val_len);

    int Get (char const *key, size_t const key_len,
	     char *val, size_t &val_len);

    int FindOrCreate (char const *key, size_t const key_len,
         char const *val, size_t const val_len,
        char *ret_val, size_t &ret_len);

    int Del (char const *key, size_t const key_len);

    int Scan (int &iter_handle,
              char *key, size_t &key_len,
              char *val, size_t &val_len,
              char const *begin_key, size_t const begin_key_len,
              bool const begin_key_inclusive,
              char const *end_key, size_t const end_key_len,
              bool const end_key_inclusive);

    int GetNext(int iter_handle,
                char *key, size_t &key_len,
                char *val, size_t &val_len);

    Gptr Location () {return root_;}

    size_t MaxKeyLen() {return kMaxKeyLen;}
    size_t MaxValLen() {return kMaxValLen;}


    /*
      for consistent DRAM caching

      val_ptr is always the up-to-date val_ptr in FAM
    */
    // for non-cached put
    int Put (char const *key, size_t const key_len,
	     char const *val, size_t const val_len,
             Gptr &key_ptr, TagGptr &val_ptr);

    // for cached put
    int Put (Gptr const key_ptr, TagGptr &val_ptr,
	     char const *val, size_t const val_len);

    // for non-cached Get
    int Get (char const *key, size_t const key_len,
	     char *val, size_t &val_len,
             Gptr &key_ptr, TagGptr &val_ptr);

    // for cached Get
    // set get_value to true if one wants to fetch the value regardlessly (for shortcut caching)
    // otherwise, only when the given val_ptr is stale we fetch the value from FAM (for full caching)
    // return 1 if the val_ptr is up-to-date
    int Get (Gptr const key_ptr, TagGptr &val_ptr,
	     char *val, size_t &val_len, bool get_value=false);

    // for non-cached Del
    int Del (char const *key, size_t const key_len,
             Gptr &key_ptr, TagGptr &val_ptr);

    // for cached Del
    int Del (Gptr const key_ptr, TagGptr &val_ptr);

    void ReportMetrics();

private:
    struct ValBuf {
        size_t size;
        char val[0];
    };

    nvmm::PoolId heap_id_;
    size_t heap_size_;

    Mmgr *mmgr_;
    Emgr *emgr_;
    Heap *heap_;

    RadixTree *tree_;
    Gptr root_;
    KVSMetrics *metrics_;

    // TODO: remove mutex?
    std::mutex mutex_; // for iterator deque
    // TODO: clean up used iters
    //std::vector<RadixTree::Iter*> iters_;
    std::deque<RadixTree::Iter*> iters_;


    int Open();
    int Close();

};

} // namespace radixtree

#endif
