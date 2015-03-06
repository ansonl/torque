#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <iostream>
#include "trq_cgroups.h"
#include <check.h>
#include <string>
#include "pbs_error.h"

#define LOGLEVEL   6

void trq_cg_init_subsys_online();
int init_subsystems(std::string& sub_token, std::string& mount_point); 
extern int create_cgroup_hierarchy();

START_TEST(test_trq_cg_init_subsys_online)
  {
  trq_cg_init_subsys_online();
  }
END_TEST

START_TEST(test_trq_cg_initialize_hierarchy)
  {
  char buf[256];
  int rc;
  std::string  cgroup_path;
  std::string  tmp_cgroup_dir;
  struct stat  stat_buf;

  tmp_cgroup_dir = "/tmp/cgroup";
  rmdir(tmp_cgroup_dir.c_str());
  
  rc = create_cgroup_hierarchy();
  fail_unless(rc == 0, "Could not create cgroup hierarchy");

  rc = trq_cg_get_cgroup_paths_from_file();
  fail_unless(rc != 0, "trq_cg_get_cgroup_paths_from_file failed");


  cgroup_path = PBS_SERVER_HOME;
  cgroup_path = cgroup_path + "/trq-cgroup-paths";

  rc = stat(cgroup_path.c_str(), &stat_buf);
  if (rc != 0)
    {
    /* the trq-cgroup-paths file does not exist. create one */
    std::ofstream cgroup_file(cgroup_path.c_str());

    cgroup_file << "cpu /tmp/cgroup/cpu\n";
    cgroup_file << "cpuset /tmp/cgroup/cpuset\n";
    cgroup_file << "cpuacct /tmp/cgroup/cpuacct\n";
    cgroup_file << "memory /tmp/cgroup/memory\n";
    cgroup_file << "devices /tmp/cgroup/devices\n";

    cgroup_file.close();

    }
    
  /* test trq_cg_initialize_hierarchy when there is a configuration
   * file present */
  rc = trq_cg_initialize_hierarchy();
  fail_unless(rc == 0);

  rc = remove(cgroup_path.c_str());
  fail_unless(rc == 0, "remove didn't work");
  

  /* test with default cgroups */
  rc = trq_cg_initialize_hierarchy();
  sprintf(buf, "Failed to initialize hierarchy: %d\n", rc);
  fail_unless(rc==0, buf);


  }
END_TEST

START_TEST(test_init_subsystems)
  {
  int  rc;
  std::string mount_point;
  std::string subsystem;

  mount_point = "/tmp/cgroup/cpu";
  subsystem = "cpu";
  rc = init_subsystems(subsystem, mount_point);
  fail_unless(rc == 0, "init subsystem failed");

  mount_point = "/tmp/cgroup/cpuacct";
  subsystem = "cpuacct";
  rc = init_subsystems(subsystem, mount_point);
  fail_unless(rc == 0, "cpuacct");

  mount_point = "/tmp/cgroup/cpuset";
  subsystem = "cpuset";
  rc = init_subsystems(subsystem, mount_point);
  fail_unless(rc == 0, "cpuset");

  mount_point = "/tmp/cgroup/memory";
  subsystem = "memory";
  rc = init_subsystems(subsystem, mount_point);
  fail_unless(rc == 0, "memory");

  mount_point = "/tmp/cgroup/devices";
  subsystem = "devices";
  rc = init_subsystems(subsystem, mount_point);
  fail_unless(rc == 0, "devices");

  }
END_TEST



START_TEST(test_trq_cg_set_resident_memory_limit)
  {
  pid_t pid = 1234;
  unsigned long mem = 1024*1024;
  std::string mem_path;
  std::string cgroup_path;
  int rc;
  FILE *fd;

  cg_memory_path = "/tmp/cgroup/memory/";

  rc = trq_cg_set_resident_memory_limit(pid, mem);
  fail_unless(rc == PBSE_SYSTEM);

  cgroup_path = "/tmp/cgroup/";
  mkdir(cgroup_path.c_str(), 0755);

  mkdir(cg_memory_path.c_str(), 0755);
  mem_path = cg_memory_path + "1234";
  mkdir(mem_path.c_str(), 0755);
  mem_path = mem_path + "/memory.limit_in_bytes";
  fd = fopen(mem_path.c_str(), "w");
  fclose(fd);
  rc = trq_cg_set_resident_memory_limit(pid, mem);
  fail_unless(rc == PBSE_NONE);
  remove(mem_path.c_str());
  mem_path = cg_memory_path + "1234";
  remove(mem_path.c_str());
  remove(cg_memory_path.c_str());
  remove(cgroup_path.c_str());


  }
END_TEST

START_TEST(test_trq_cg_set_swap_memory_limit)
  {
  pid_t pid = 1234;
  unsigned long mem = 1024*1024;
  std::string mem_path;
  int rc;
  FILE *fd;

  cg_memory_path = "/tmp/cgroup/";

  rc = trq_cg_set_swap_memory_limit(pid, mem);
  fail_unless(rc == PBSE_SYSTEM);

  mkdir(cg_memory_path.c_str(), 0755);
  mem_path = cg_memory_path + "1234";
  mkdir(mem_path.c_str(), 0755);
  mem_path = mem_path + "/memory.memsw.limit_in_bytes";
  fd = fopen(mem_path.c_str(), "w");
  fclose(fd);
  rc = trq_cg_set_swap_memory_limit(pid, mem);
  fail_unless(rc == PBSE_NONE);
  remove(mem_path.c_str());
  mem_path = cg_memory_path + "1234";
  remove(mem_path.c_str());
  remove(cg_memory_path.c_str());


  }
END_TEST




Suite *trq_cgroups_suite(void)
  {
  Suite *s = suite_create("trq_cgroups test suite methods");
  TCase *tc_core = tcase_create("test_trq_cg_init_subsys_online");
  tcase_add_test(tc_core, test_trq_cg_init_subsys_online);
  suite_add_tcase(s, tc_core);
  
  tc_core = tcase_create("test_trq_cg_initialize_hierarchy");
  tcase_add_test(tc_core, test_trq_cg_initialize_hierarchy);
  suite_add_tcase(s, tc_core);
  
  tc_core = tcase_create("test_init_subsystems");
  tcase_add_test(tc_core, test_init_subsystems);
  suite_add_tcase(s, tc_core);
  
  tc_core = tcase_create("test_trq_cg_set_resident_memory_limit");
  tcase_add_test(tc_core, test_trq_cg_set_resident_memory_limit);
  suite_add_tcase(s, tc_core);
  
  tc_core = tcase_create("test_trq_cg_set_swap_memory_limit");
  tcase_add_test(tc_core, test_trq_cg_set_swap_memory_limit);
  suite_add_tcase(s, tc_core);
  
 
  return(s);
  }

void rundebug()
  {
  }

int main(void)
  {
  int number_failed = 0;
  SRunner *sr = NULL;
  rundebug();
  sr = srunner_create(trq_cgroups_suite());
  srunner_set_log(sr, "trq_cgroups_suite.log");
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return(number_failed);
  }