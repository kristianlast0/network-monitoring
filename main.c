#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include "db.h"

void process_packet(const struct ip *iph, const struct tcphdr *tcph);

void packet_handler(unsigned char *user_data, const struct pcap_pkthdr *pkthdr, const unsigned char *packet) {
    // printf("Packet captured %s\n", packet);
    struct ip *iph = (struct ip*)(packet + 14); // Assuming Ethernet frame
    struct tcphdr *tcph = (struct tcphdr*)(packet + 14 + iph->ip_hl * 4);
    process_packet(iph, tcph);
}

void process_packet(const struct ip *iph, const struct tcphdr *tcph) {

    uint32_t source_ip = ntohl(iph->ip_src.s_addr);
    uint16_t source_port = ntohs(tcph->th_sport);
    uint32_t destination_ip = ntohl(iph->ip_dst.s_addr);
    uint16_t destination_port = ntohs(tcph->th_dport);

    if (tcph->syn && !tcph->ack) {
        // SYN packet (connection initiation)
        if (!connection_exists(source_ip, source_port, destination_ip, destination_port)) {
            // New connection, allow and update state table
            printf("New connection established: %u:%u -> %u:%u\n", source_ip, source_port, destination_ip, destination_port);
            insert_connection(source_ip, source_port, destination_ip, destination_port);
        } else {
            // Existing connection, update state table
            printf("Connection already established: %u:%u -> %u:%u\n", source_ip, source_port, destination_ip, destination_port);
        }
    }
    else if (tcph->fin) {
        // FIN packet (connection closure)
        if (connection_exists(source_ip, source_port, destination_ip, destination_port)) {
            printf("FIN packet detected: %u:%u -> %u:%u\n", source_ip, source_port, destination_ip, destination_port);
            delete_connection(source_ip, source_port, destination_ip, destination_port);
        }
    }
}

int main(int argc, char *argv[]) {

    // init_hash_table();
    init_database();

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <interface>\n", argv[0]);
        return 1;
    }

    handle = pcap_open_live(argv[1], BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Could not open device %s: %s\n", argv[1], errbuf);
        return 2;
    }

    struct bpf_program fp;
    char filter_exp[] = "tcp";
    if (pcap_compile(handle, &fp, filter_exp, 0, PCAP_NETMASK_UNKNOWN) == -1) {
        fprintf(stderr, "Could not parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return 2;
    }
    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "Could not install filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return 2;
    }

    pcap_loop(handle, 0, packet_handler, NULL);
    pcap_close(handle);

    cleanup_database();

    return 0;
}