/*
 * Copyright (c) 2003 Sun Microsystems, Inc.  All Rights Reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistribution of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * Redistribution in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of Sun Microsystems, Inc. or the names of
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * 
 * This software is provided "AS IS," without a warranty of any kind.
 * ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED.
 * SUN MICROSYSTEMS, INC. ("SUN") AND ITS LICENSORS SHALL NOT BE LIABLE
 * FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.  IN NO EVENT WILL
 * SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA,
 * OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR
 * PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY OF
 * LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE,
 * EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 */

#pragma once

#include <ipmitool/ipmi.h>
#include <ipmitool/ipmi_oem.h>
#include <ipmitool/ipmi_constants.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*
 * An enumeration that describes every possible session state for
 * an IPMIv2 / RMCP+ session.
 */
enum LANPLUS_SESSION_STATE {
	LANPLUS_STATE_PRESESSION = 0,
	LANPLUS_STATE_OPEN_SESSION_SENT,
	LANPLUS_STATE_OPEN_SESSION_RECEIEVED,
	LANPLUS_STATE_RAKP_1_SENT,
	LANPLUS_STATE_RAKP_2_RECEIVED,
	LANPLUS_STATE_RAKP_3_SENT,
	LANPLUS_STATE_ACTIVE,
	LANPLUS_STATE_CLOSE_SENT,
};


#define IPMI_AUTHCODE_BUFFER_SIZE 20
#define IPMI_SIK_BUFFER_SIZE      IPMI_MAX_MD_SIZE
#define IPMI_KG_BUFFER_SIZE       21 /* key plus null byte */

enum cipher_suite_ids {
	IPMI_LANPLUS_CIPHER_SUITE_0 = 0,
	IPMI_LANPLUS_CIPHER_SUITE_1 = 1,
	IPMI_LANPLUS_CIPHER_SUITE_2 = 2,
	IPMI_LANPLUS_CIPHER_SUITE_3 = 3,
	IPMI_LANPLUS_CIPHER_SUITE_4 = 4,
	IPMI_LANPLUS_CIPHER_SUITE_5 = 5,
	IPMI_LANPLUS_CIPHER_SUITE_6 = 6,
	IPMI_LANPLUS_CIPHER_SUITE_7 = 7,
	IPMI_LANPLUS_CIPHER_SUITE_8 = 8,
	IPMI_LANPLUS_CIPHER_SUITE_9 = 9,
	IPMI_LANPLUS_CIPHER_SUITE_10 = 10,
	IPMI_LANPLUS_CIPHER_SUITE_11 = 11,
	IPMI_LANPLUS_CIPHER_SUITE_12 = 12,
	IPMI_LANPLUS_CIPHER_SUITE_13 = 13,
	IPMI_LANPLUS_CIPHER_SUITE_14 = 14,
#ifdef HAVE_CRYPTO_SHA256
	IPMI_LANPLUS_CIPHER_SUITE_15 = 15,
	IPMI_LANPLUS_CIPHER_SUITE_16 = 16,
	IPMI_LANPLUS_CIPHER_SUITE_17 = 17,
#endif /* HAVE_CRYPTO_SHA256 */
	IPMI_LANPLUS_CIPHER_SUITE_RESERVED = 0xff,
};

struct cipher_suite_info {
	enum cipher_suite_ids cipher_suite_id;
	uint8_t auth_alg;
	uint8_t integrity_alg;
	uint8_t crypt_alg;
	uint32_t iana;
};

struct ipmi_session_params {
	char * hostname;
	uint8_t username[17];
	uint8_t authcode_set[IPMI_AUTHCODE_BUFFER_SIZE + 1];
	uint8_t authtype_set;
	uint8_t privlvl;
	enum cipher_suite_ids cipher_suite_id;
	char sol_escape_char;
	int password;
	int port;
	int retry;
	uint32_t timeout;
	uint8_t kg[IPMI_KG_BUFFER_SIZE];   /* BMC key */
	uint8_t lookupbit;
};

#define IPMI_AUTHSTATUS_PER_MSG_DISABLED	0x10
#define IPMI_AUTHSTATUS_PER_USER_DISABLED	0x08
#define IPMI_AUTHSTATUS_NONNULL_USERS_ENABLED	0x04
#define IPMI_AUTHSTATUS_NULL_USERS_ENABLED	0x02
#define IPMI_AUTHSTATUS_ANONYMOUS_USERS_ENABLED	0x01

struct ipmi_session {
	int active;
	uint32_t session_id;
	uint32_t in_seq;
	uint32_t out_seq;

	uint8_t authcode[IPMI_AUTHCODE_BUFFER_SIZE + 1];
	uint8_t challenge[16];
	uint8_t authtype;
	uint8_t authstatus;
	uint8_t authextra;
	uint32_t timeout;

	struct sockaddr_storage addr;
	socklen_t addrlen;

	/*
	 * This struct holds state data specific to IPMI v2 / RMCP+ sessions
	 */
	struct {
		enum LANPLUS_SESSION_STATE session_state;

		/* These are the algorithms agreed upon for the session */
		uint8_t requested_auth_alg;
		uint8_t requested_integrity_alg;
		uint8_t requested_crypt_alg;
		uint8_t auth_alg;
		uint8_t integrity_alg;
		uint8_t crypt_alg;
		uint8_t max_priv_level;

		uint32_t console_id;
		uint32_t bmc_id;

		/*
		 * Values required for RAKP messages
		 */

		/* Random number generated byt the console */
		uint8_t console_rand[16]; 
		/* Random number generated by the BMC */
		uint8_t bmc_rand[16];

		uint8_t bmc_guid[16];
		uint8_t requested_role;   /* As sent in the RAKP 1 message */
		uint8_t rakp2_return_code;

		uint8_t  sik[IPMI_SIK_BUFFER_SIZE]; /* Session integrity key */
		uint8_t sik_len;                   /* Session Integrity key length */
		uint8_t  kg[IPMI_KG_BUFFER_SIZE];   /* BMC key */
		uint8_t  k1[IPMI_MAX_MD_SIZE];      /* Used for Integrity checking? */
		uint8_t k1_len;                    /* K1 key length */
		uint8_t  k2[IPMI_MAX_MD_SIZE];      /* First 16 bytes used for AES  */
		uint8_t k2_len;                    /* K2 key length */
	} v2_data;


	/*
	 * This data is specific to the Serial Over Lan session
	 */
	struct {
		uint16_t max_inbound_payload_size;
		uint16_t max_outbound_payload_size;
		uint16_t port;
		uint8_t sequence_number;

		/*  This data describes the last SOL packet */
		uint8_t last_received_sequence_number;
		uint8_t last_received_byte_count;
		void (*sol_input_handler)(struct ipmi_rs * rsp);
	} sol_data;
};

struct ipmi_cmd {
	int (*func)(struct ipmi_intf * intf, int argc, char ** argv);
	const char * name;
	const char * desc;
};

struct ipmi_intf_support {
	const char * name;
	int supported;
};

struct ipmi_intf {
	char name[16];
	char desc[128];
	char *devfile;
	int fd;
	int opened;
	int abort;
	int noanswer;
	int picmg_avail;
	int vita_avail;
	IPMI_OEM manufacturer_id;
	int ai_family;

	struct ipmi_session_params ssn_params;
	struct ipmi_session * session;
	struct ipmi_oem_handle * oem;
	struct ipmi_cmd * cmdlist;
	uint8_t	target_ipmb_addr;
	uint32_t my_addr;
	uint32_t target_addr;
	uint8_t target_lun;
	uint8_t target_channel;
	uint32_t transit_addr;
	uint8_t transit_channel;
	uint16_t max_request_data_size;
	uint16_t max_response_data_size;

	uint8_t devnum;

	int (*setup)(struct ipmi_intf * intf);
	int (*open)(struct ipmi_intf * intf);
	void (*close)(struct ipmi_intf * intf);
	struct ipmi_rs *(*sendrecv)(struct ipmi_intf * intf, struct ipmi_rq * req);
	struct ipmi_rs *(*recv_sol)(struct ipmi_intf * intf);
	struct ipmi_rs *(*send_sol)(struct ipmi_intf * intf, struct ipmi_v2_payload * payload);
	int (*keepalive)(struct ipmi_intf * intf);
	int (*set_my_addr)(struct ipmi_intf * intf, uint8_t addr);
	void (*set_max_request_data_size)(struct ipmi_intf * intf, uint16_t size);
	void (*set_max_response_data_size)(struct ipmi_intf * intf, uint16_t size);
};

uint16_t ipmi_intf_get_max_request_data_size(struct ipmi_intf *intf);
uint16_t ipmi_intf_get_max_response_data_size(struct ipmi_intf *intf);
uint8_t ipmi_intf_get_bridging_level(const struct ipmi_intf *intf);

struct ipmi_intf * ipmi_intf_load(char * name);
void ipmi_intf_print(struct ipmi_intf_support * intflist);

void ipmi_intf_session_set_hostname(struct ipmi_intf * intf, char * hostname);
void ipmi_intf_session_set_username(struct ipmi_intf * intf, char * username);
void ipmi_intf_session_set_password(struct ipmi_intf * intf, char * password);
void ipmi_intf_session_set_privlvl(struct ipmi_intf * intf, uint8_t privlvl);
void ipmi_intf_session_set_lookupbit(struct ipmi_intf * intf, uint8_t lookupbit);
#ifdef IPMI_INTF_LANPLUS
void ipmi_intf_session_set_cipher_suite_id(struct ipmi_intf * intf,
                                           enum cipher_suite_ids cipher_suite_id);
#endif /* IPMI_INTF_LANPLUS */
void ipmi_intf_session_set_sol_escape_char(struct ipmi_intf * intf, char sol_escape_char);
void ipmi_intf_session_set_kgkey(struct ipmi_intf *intf, const uint8_t *kgkey);
void ipmi_intf_session_set_port(struct ipmi_intf * intf, int port);
void ipmi_intf_session_set_authtype(struct ipmi_intf * intf, uint8_t authtype);
void ipmi_intf_session_set_timeout(struct ipmi_intf * intf, uint32_t timeout);
void ipmi_intf_session_set_retry(struct ipmi_intf * intf, int retry);
void ipmi_intf_session_cleanup(struct ipmi_intf *intf);
void ipmi_cleanup(struct ipmi_intf * intf);

#if defined(IPMI_INTF_LAN) || defined (IPMI_INTF_LANPLUS)
int  ipmi_intf_socket_connect(struct ipmi_intf * intf);
#endif
