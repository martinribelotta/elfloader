#ifndef LOADER_USER_DATA_H
#define LOADER_USER_DATA_H

typedef struct loader_env {
  int fd;
  const struct ELFEnv * env;
} loader_env_t;

#define LOADER_USERDATA_T loader_env_t

#endif
