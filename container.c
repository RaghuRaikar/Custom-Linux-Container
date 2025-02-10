#define _GNU_SOURCE

#include <err.h>
#include <errno.h>
#include <linux/limits.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <wait.h>

#include "change_root.h"

#define CONTAINER_ID_MAX 16
#define CHILD_STACK_SIZE 4096 * 10

typedef struct container {
  char id[CONTAINER_ID_MAX];
  // TODO: Add fields
  // here we create a container_image for our container
  char container_image[PATH_MAX];
  // here we store the arguments for the container
  char** container_args;
} container_t;

/**
 * `usage` prints the usage of `client` and exists the program with
 * `EXIT_FAILURE`
 */
void usage(char* cmd) {
  printf("Usage: %s [ID] [IMAGE] [CMD]...\n", cmd);
  exit(EXIT_FAILURE);
}

/**
 * `container_exec` is an entry point of a child process and responsible for
 * creating an overlay filesystem, calling `change_root` and executing the
 * command given as arguments.
 */
int container_exec(void* arg) {
  container_t* container = (container_t*)arg;

  // this line is required on some systems
  if (mount("/", "/", "none", MS_PRIVATE | MS_REC, NULL) < 0) {
    err(1, "mount / private");
  }

  // TODO: Create a overlay filesystem
  // `lowerdir`  should be the image directory: ${cwd}/images/${image}
  // `upperdir`  should be `/tmp/container/{id}/upper`
  // `workdir`   should be `/tmp/container/{id}/work`
  // `merged`    should be `/tmp/container/{id}/merged`
  // ensure all directories exist (create if not exists) and
  // call `mount("overlay", merged, "overlay", MS_RELATIME,
  //    lowerdir={lowerdir},upperdir={upperdir},workdir={workdir})`

  // here we create arrays of size PATH_MAX to store the directory paths
  char container_dir[PATH_MAX];
  char lowerdir[PATH_MAX];
  char upperdir[PATH_MAX];
  char workdir[PATH_MAX];
  char merged[PATH_MAX];
  // here we make sure to get the current working directory
  char* current_dir = getcwd(NULL, 0);

  // here we use the snprintf function to properly format the path we need
  snprintf(container_dir, PATH_MAX, "/tmp/container/%s", container->id);
  snprintf(lowerdir, PATH_MAX, "%s/images/%s", current_dir,
           container->container_image);
  snprintf(upperdir, PATH_MAX, "/tmp/container/%s/upper", container->id);
  snprintf(workdir, PATH_MAX, "/tmp/container/%s/work", container->id);
  snprintf(merged, PATH_MAX, "/tmp/container/%s/merged", container->id);

  // Added this block to create the container directory first
  // here we make the directories we need and also handle errors
  if (mkdir(container_dir, 0700) < 0 && errno != EEXIST) {
    free(current_dir);
    // if there is an error we print this message to the terminal
    err(1, "Failed to create the container directory");
  }

  if (mkdir(upperdir, 0700) < 0 && errno != EEXIST) {
    free(current_dir);
    // if there is an error we print this message to the terminal
    err(1, "Failed to create the upper directory");
  }

  if (mkdir(workdir, 0700) < 0 && errno != EEXIST) {
    free(current_dir);
    // if there is an error we print this message to the terminal
    err(1, "Failed to create the work directory");
  }

  // Added this block to create the merged directory before mounting
  if (mkdir(merged, 0700) < 0 && errno != EEXIST) {
    free(current_dir);
    // if there is an error we print this message to the terminal
    err(1, "Failed to create the merged directory");
  }

  // here we create a char array called all_dir to store the value of the last
  // argument to the mount function
  char all_dir[PATH_MAX];
  // we use snprintf again to make sure the char array has the proper path
  snprintf(all_dir, PATH_MAX, "lowerdir=%s,upperdir=%s,workdir=%s", lowerdir,
           upperdir, workdir);

  // here we call mount and pass all the needed requirements and also use error
  // handling incase it fails
  if (mount("overlay", merged, "overlay", MS_RELATIME, all_dir) < 0) {
    free(current_dir);
    // if there is an error we print this message to the terminal
    err(1, "Failed to mount the overlay filesystem");
  }

  // TODO: Call `change_root` with the `merged` directory
  // change_root(merged)
  change_root(merged);

  // TODO: use `execvp` to run the given command and return its return value

  // here we point to the address of first element of container_args
  char** container_args_copy = &container->container_args[0];
  free(current_dir);
  // the double pointer allows us to pass in first element and also list of args
  // into execvp
  if (execvp(container_args_copy[0], container_args_copy) < 0) {
    // if there is an error we print this message to the terminal
    err(1, "Failed to execute execvp");
  }

  return 0;
}

int main(int argc, char** argv) {
  if (argc < 4) {
    usage(argv[0]);
  }

  /* Create tmpfs and mount it to `/tmp/container` so overlayfs can be used
   * inside Docker containers */
  if (mkdir("/tmp/container", 0700) < 0 && errno != EEXIST) {
    err(1, "Failed to create a directory to store container file systems");
  }
  if (errno != EEXIST) {
    if (mount("tmpfs", "/tmp/container", "tmpfs", 0, "") < 0) {
      err(1, "Failed to mount tmpfs on /tmp/container");
    }
  }

  /* cwd contains the absolute path to the current working directory which can
   * be useful constructing path for container_image */
  char cwd[PATH_MAX];
  getcwd(cwd, PATH_MAX);

  container_t container;
  strncpy(container.id, argv[1], CONTAINER_ID_MAX);
  // TODO: store all necessary information to `container`
  // here we copy the name of the image into the container_image variable
  strncpy(container.container_image, argv[2], PATH_MAX);
  // here we store all the arguments in the container_args variable
  container.container_args = &argv[3];

  /* Use `clone` to create a child process */
  char child_stack[CHILD_STACK_SIZE];  // statically allocate stack for child
  int clone_flags = SIGCHLD | CLONE_NEWNS | CLONE_NEWPID;
  int pid = clone(container_exec, &child_stack, clone_flags, &container);
  if (pid < 0) {
    err(1, "Failed to clone");
  }

  waitpid(pid, NULL, 0);
  return EXIT_SUCCESS;
}
