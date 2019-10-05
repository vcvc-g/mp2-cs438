#include <sender_helper.h>

#define ALPHA 0.125
#define BETA  0.25
#define SAFETY_MARGIN 15
#define SLOW_START 0
#define CONGESTION_AVOID 1
#define FAST_RECOVERY 2

/* 
Function timeout_interval(): calculate timeout interval for sending packet;
float estimated_rtt: rtt from last timeout_interval call;
float sampled_rtt: measured rtt from time();
*/
float timeout_interval(float estimated_rtt, float sampled_rtt) {
    // using hard code safty margin for now
    // dev_rtt = (1-BETA)*dev_rtt + BETA * abs(sampled_rtt-estimated_rtt);
    return (1-ALPHA)*estimated_rtt + ALPHA*sampled_rtt + 4*SAFETY_MARGIN;
}

void adjust_window(struct sender_info *sender){
    int cur_state = sender->congestion_state; 
    if (cur_state == SLOW_START) {
        sender->window_size+=1;
    }

    else if (cur_state == CONGESTION_AVOID) {
        sender->window_size+=1;
    }

    else if (cur_state == FAST_RECOVERY) {
        sender->window_size*=2;
    }

}
