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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <gtest/gtest.h>
#include <random>

#include "cluster/config.h"
#include "cluster/cluster.h"

using namespace radixtree;

TEST(Cluster, loadConfigfile) {
    std::string path = "test_cluster.yaml";

    // load config file
    Config config;
    //config.PrintConfigFile(path);
    config.LoadConfigFile(path);
    assert(config.IsValid());

    // test the cluster
    Cluster c;
    c.Init(config);
    c.Print();
}

// TEST(Cluster, updateConfigfile) {
//     // load config file
//     std::string path = "test_cluster.yaml";
//     Config config;
//     //config.PrintConfigFile(path);
//     config.LoadConfigFile(path);
//     assert(config.IsValid());

//     // save as another file
//     std::string new_path = "test_cluster_new.yaml";
//     config.SaveConfigFile(new_path);
// }

// TEST(Cluster, routing) {
//     std::string path = "test_cluster.yaml";

//     // load config file
//     Config config;
//     //config.PrintConfigFile(path);
//     config.LoadConfigFile(path);
//     assert(config.IsValid());

//     // test the cluster
//     Cluster c;
//     c.Init(config);
//     c.Print();

//     // ResourceName r1("hello");
//     // Location l1;
//     // for(int i=0; i<10; i++) {
//     //     l1 = c.Locate(r1);
//     //     std::cout << r1 << " -> " << l1 << std::endl;
//     // }

//     // ResourceName r2("world");
//     // Location l2;
//     // l2 = c.Locate(r2);
//     // for(int i=0; i<10; i++) {
//     //     std::cout << r2 << " -> " << l2 << std::endl;
//     //     l2 = c.Locate(r2);
//     // }
// }

void Init()
{
}

int main (int argc, char** argv) {
    Init();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
