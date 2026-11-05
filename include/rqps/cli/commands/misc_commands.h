#pragma once

namespace rqps::cli {

int cmd_init();

int cmd_version();

int cmd_policy_show();

#ifdef RQPS_HAVE_HTTP
int cmd_remote(int argc, char* argv[]);

int cmd_ping(int argc, char* argv[]);

int cmd_serve(int argc, char* argv[]);
#endif

int cmd_http_unavailable();

}  // namespace rqps::cli
