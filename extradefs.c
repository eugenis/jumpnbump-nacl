#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <signal.h>

// Poor man's libnosys.a

uid_t getuid(void) {
  return 0;
}

uid_t geteuid(void) {
  return 0;
}

gid_t getgid(void) {
  return 0;
}

gid_t getegid(void) {
  return 0;
}

int setuid(uid_t uid) {
  errno = -EPERM;
  return -1;
}

struct passwd *getpwnam(const char *name) {
  return NULL;
}
