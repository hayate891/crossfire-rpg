#include <unistd.h>

#include "toolkit_server.h"

void clean_test_account_data(void) {
    unlink("/tmp/account/testaccount");
    rmdir("/tmp/account");
    unlink("/tmp/accounts");
}
