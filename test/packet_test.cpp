#include <string>
#include "packet.h"
#include <iostream>
#include <cassert>

int main(int argc, char * * argv) {
    std::string msg("When in the Course of human events, it becomes necessary for one people to dissolve the political bands which have connected them with another, and to assume, among the Powers of the earth, the separate and equal station to which the Laws of Nature and of Nature's God entitle them, a decent respect to the opinions of mankind requires that they should declare the causes which impel them to the separation.");
    std::string recip("King George III");
    std::string sender("John Hancock et al.");
    messageSend out(msg, recip);

    unsigned char buffer[PACKET_BUFFER_SIZE];
    int amt = out.write_to_packet(buffer);
    messageSend in(buffer);

    while (amt) {
        amt = out.write_to_packet(buffer);
        in.read_from_packet(buffer);
    }

    assert(in.recipient == recip);
    assert(in.message == msg);

    std::string msg2("Hello");
    std::string recip2("Watson");
    std::string sender2("Bell");
    messageSend out2(msg2, recip2);

    amt = out2.write_to_packet(buffer);
    messageSend in2(buffer);

    while (amt) {
        amt = out2.write_to_packet(buffer);
        in2.read_from_packet(buffer);
    }

    assert(in2.recipient == recip2);
    assert(in2.message == msg2);

    messageForward fwd(msg, sender);
    amt = fwd.write_to_packet(buffer);
    messageForward lnd(buffer);

    while (amt) {
        amt = fwd.write_to_packet(buffer);
        lnd.read_from_packet(buffer);
    }

    assert(lnd.sender == sender);
    assert(lnd.message == msg);

    messageForward fwd2(msg2, sender2);
    amt = fwd2.write_to_packet(buffer);
    messageForward lnd2(buffer);

    while (amt) {
        amt = fwd2.write_to_packet(buffer);
        lnd2.read_from_packet(buffer);
    }

    assert(lnd2.sender == sender2);
    assert(lnd2.message == msg2);

    return 0;
}