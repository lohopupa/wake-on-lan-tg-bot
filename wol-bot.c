#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


char *TG_BOT_TOKEN;
char *TARGET_IP;
char *TARGET_MAC;
char *WOL_COMMAND;
char *CHECK_STATUS_COMMAND;
long ALLOWD_CHAT_ID;


#define DEBUG(fmt, ...) fprintf(stdout, "[DEBUG]: " fmt "\n", ##__VA_ARGS__)
#define INFO(fmt, ...) fprintf(stdout, "[INFO]: " fmt "\n", ##__VA_ARGS__)
#define ERROR(fmt, ...) fprintf(stderr, "[ERROR]: " fmt "\n", ##__VA_ARGS__)

bool check_chat_id(long chat_id) { return ALLOWD_CHAT_ID == chat_id; }

void send_message(long chat_id, const char *text) {
    char url[2048];
    snprintf(url, sizeof(url),
             "https://api.telegram.org/bot%s/sendMessage?chat_id=%ld&text=%s",
             TG_BOT_TOKEN, chat_id, text);

    char command[4096];
    snprintf(command, sizeof(command), "wget -qO- '%s'", url);

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        ERROR("Failed to send message");
        exit(1);
    }
    pclose(fp);
}

void handle_update(const char *response, long *last_update) {
    char *p = strstr(response, "\"update_id\":");
    if (p) {
        long update_id = atol(p + 12);
        if (update_id > *last_update)
            *last_update = update_id;
        char *cp = strstr(response, "\"chat\":{\"id\":");
        if (cp) {
            long chat_id = atol(cp + 13);
            char *tp = strstr(response, "\"text\":\"");
            if (tp) {
                tp += 8;
                char *ep = strchr(tp, '\"');
                if (ep) {
                    int text_len = ep - tp;
                    char *text = malloc(text_len + 1);
                    strncpy(text, tp, text_len);
                    text[text_len] = '\0';
                    if (check_chat_id(chat_id)) {
                        if (strncmp(text, "/status", 7) == 0) {
                            int reachable = system(CHECK_STATUS_COMMAND) == 0;
                            send_message(chat_id, reachable ? "🟢 Online" : "🔴 Offline");
                        } else if (strncmp(text, "/wakeup", 7) == 0) {
                            int ret = system(WOL_COMMAND);
                            if (ret == 0) {
                                INFO("Sent signal to pc");
                                send_message(chat_id, "🟢 Woken up");
                            } else {
                                ERROR("Could not send signal to pc");
                                send_message(chat_id, "🔴 Failed to execute command");
                            }
                        } else {
                            send_message(chat_id, "Usage:\n/status - To get current status of PC\n/wakeup - To wake up the PC\n");
                        }
                    } else {
                        INFO("WARNING DISALLOWED USER: %ld", chat_id);
                        char *text = malloc(256);
                        int n = sprintf(text, "🔴🔴User (%ld) is trying to fuck me🔴🔴", chat_id);
                        text[n] = '\0';
                        send_message(ALLOWD_CHAT_ID, text);
                        free(text);
                    }
                    free(text);
                }
            }
        }
    }
}

void read_env_vars() {
    TG_BOT_TOKEN = getenv("TELEGRAM_BOT_TOKEN");
    if (!TG_BOT_TOKEN) {
        ERROR("Could not get TELEGRAM_BOT_TOKEN");
        exit(1);
    }

    TARGET_IP = getenv("TARGET_IP");
    if (!TARGET_IP) {
        ERROR("Could not get TARGET_IP");
        exit(1);
    }

    TARGET_MAC = getenv("TARGET_MAC");
    if (!TARGET_MAC) {
        ERROR("Could not get TARGET_MAC");
        exit(1);
    }

    char *wol_command_template = getenv("WOL_COMMAND");
    if (!wol_command_template) {
        ERROR("Could not get WOL_COMMAND");
        exit(1);
    }

    char *check_status_command_template = getenv("CHECK_STATUS_COMMAND");
    if (!check_status_command_template) {
        ERROR("Could not get CHECK_STATUS_COMMAND");
        exit(1);
    }

    WOL_COMMAND = malloc(1024);
    CHECK_STATUS_COMMAND = malloc(1024);
    sprintf(WOL_COMMAND, wol_command_template, TARGET_MAC);
    sprintf(CHECK_STATUS_COMMAND, check_status_command_template, TARGET_IP);

    char *allowed_chat_id_str = getenv("ALLOWED_CHAT_ID");
    if (!allowed_chat_id_str) {
        ERROR("Could not get ALLOWED_CHAT_ID");
        exit(1);
    }
    ALLOWD_CHAT_ID = atol(allowed_chat_id_str);
}

int main() {
    INFO("Starting bot");
    
    read_env_vars();
    
    DEBUG("TG_BOT_TOKEN: %s", TG_BOT_TOKEN);
    DEBUG("TARGET_IP: %s", TARGET_IP);
    DEBUG("TARGET_MAC: %s", TARGET_MAC);
    DEBUG("WOL_COMMAND: %s", WOL_COMMAND);
    DEBUG("CHECK_STATUS_COMMAND: %s", CHECK_STATUS_COMMAND);
    DEBUG("ALLOWD_CHAT_ID: %ld", ALLOWD_CHAT_ID);

    long last_update = 0;

    INFO("Start polling");

    while (1) {
        char command[2048];
        snprintf(command, sizeof(command), "wget -qO- 'https://api.telegram.org/bot%s/getUpdates?offset=%ld'", TG_BOT_TOKEN, last_update + 1);

        FILE *fp = popen(command, "r");
        if (fp == NULL) {
            ERROR("Failed to run command");
            exit(1);
        }

        char response[4096] = {0};
        size_t len = 0;
        while (fgets(response + len, sizeof(response) - len, fp) != NULL) {
            len = strlen(response);
        }
        pclose(fp);

        if (len > 0) {
            handle_update(response, &last_update);
        }

        sleep(1);
    }

    INFO("Exiting bot");
    return 0;
}
