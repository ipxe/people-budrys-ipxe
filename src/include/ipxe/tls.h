#ifndef _IPXE_TLS_H
#define _IPXE_TLS_H

/**
 * @file
 *
 * Transport Layer Security Protocol
 */

FILE_LICENCE ( GPL2_OR_LATER );

#include <stdint.h>
#include <ipxe/refcnt.h>
#include <ipxe/interface.h>
#include <ipxe/process.h>
#include <ipxe/crypto.h>
#include <ipxe/md5.h>
#include <ipxe/sha1.h>
#include <ipxe/sha256.h>
#include <ipxe/x509.h>

/** A TLS header */
struct tls_header {
	/** Content type
	 *
	 * This is a TLS_TYPE_XXX constant
	 */
	uint8_t type;
	/** Protocol version
	 *
	 * This is a TLS_VERSION_XXX constant
	 */
	uint16_t version;
	/** Length of payload */
	uint16_t length;
} __attribute__ (( packed ));

/** TLS version 1.0 */
#define TLS_VERSION_TLS_1_0 0x0301

/** TLS version 1.1 */
#define TLS_VERSION_TLS_1_1 0x0302

/** TLS version 1.2 */
#define TLS_VERSION_TLS_1_2 0x0303

/** Change cipher content type */
#define TLS_TYPE_CHANGE_CIPHER 20

/** Alert content type */
#define TLS_TYPE_ALERT 21

/** Handshake content type */
#define TLS_TYPE_HANDSHAKE 22

/** Application data content type */
#define TLS_TYPE_DATA 23

/* Handshake message types */
#define TLS_HELLO_REQUEST 0
#define TLS_CLIENT_HELLO 1
#define TLS_SERVER_HELLO 2
#define TLS_CERTIFICATE 11
#define TLS_SERVER_KEY_EXCHANGE 12
#define TLS_CERTIFICATE_REQUEST 13
#define TLS_SERVER_HELLO_DONE 14
#define TLS_CERTIFICATE_VERIFY 15
#define TLS_CLIENT_KEY_EXCHANGE 16
#define TLS_FINISHED 20

/* TLS alert levels */
#define TLS_ALERT_WARNING 1
#define TLS_ALERT_FATAL 2

/* TLS cipher specifications */
#define TLS_RSA_WITH_NULL_MD5 0x0001
#define TLS_RSA_WITH_NULL_SHA 0x0002
#define TLS_RSA_WITH_AES_128_CBC_SHA 0x002f
#define TLS_RSA_WITH_AES_256_CBC_SHA 0x0035
#define TLS_RSA_WITH_AES_128_CBC_SHA256 0x003c
#define TLS_RSA_WITH_AES_256_CBC_SHA256 0x003d

/* TLS hash algorithm identifiers */
#define TLS_MD5_ALGORITHM 1
#define TLS_SHA1_ALGORITHM 2
#define TLS_SHA256_ALGORITHM 4

/* TLS signature algorithm identifiers */
#define TLS_RSA_ALGORITHM 1

/* TLS extension types */
#define TLS_SERVER_NAME 0
#define TLS_SERVER_NAME_HOST_NAME 0

/** TLS RX state machine state */
enum tls_rx_state {
	TLS_RX_HEADER = 0,
	TLS_RX_DATA,
};

/** TLS TX pending flags */
enum tls_tx_pending {
	TLS_TX_CLIENT_HELLO = 0x0001,
	TLS_TX_CERTIFICATE = 0x0002,
	TLS_TX_CLIENT_KEY_EXCHANGE = 0x0004,
	TLS_TX_CERTIFICATE_VERIFY = 0x0008,
	TLS_TX_CHANGE_CIPHER = 0x0010,
	TLS_TX_FINISHED = 0x0020,
};

/** A TLS cipher suite */
struct tls_cipher_suite {
	/** Public-key encryption algorithm */
	struct pubkey_algorithm *pubkey;
	/** Bulk encryption cipher algorithm */
	struct cipher_algorithm *cipher;
	/** MAC digest algorithm */
	struct digest_algorithm *digest;
	/** Key length */
	uint16_t key_len;
	/** Numeric code (in network-endian order) */
	uint16_t code;
};

/** A TLS cipher specification */
struct tls_cipherspec {
	/** Cipher suite */
	struct tls_cipher_suite *suite;
	/** Dynamically-allocated storage */
	void *dynamic;
	/** Public key encryption context */
	void *pubkey_ctx;
	/** Bulk encryption cipher context */
	void *cipher_ctx;
	/** Next bulk encryption cipher context (TX only) */
	void *cipher_next_ctx;
	/** MAC secret */
	void *mac_secret;
};

/** A TLS signature and hash algorithm identifier */
struct tls_signature_hash_id {
	/** Hash algorithm */
	uint8_t hash;
	/** Signature algorithm */
	uint8_t signature;
} __attribute__ (( packed ));

/** A TLS signature algorithm */
struct tls_signature_hash_algorithm {
	/** Digest algorithm */
	struct digest_algorithm *digest;
	/** Public-key algorithm */
	struct pubkey_algorithm *pubkey;
	/** Numeric code */
	struct tls_signature_hash_id code;
};

/** TLS pre-master secret */
struct tls_pre_master_secret {
	/** TLS version */
	uint16_t version;
	/** Random data */
	uint8_t random[46];
} __attribute__ (( packed ));

/** TLS client random data */
struct tls_client_random {
	/** GMT Unix time */
	uint32_t gmt_unix_time;
	/** Random data */
	uint8_t random[28];
} __attribute__ (( packed ));

/** An MD5+SHA1 context */
struct md5_sha1_context {
	/** MD5 context */
	uint8_t md5[MD5_CTX_SIZE];
	/** SHA-1 context */
	uint8_t sha1[SHA1_CTX_SIZE];
} __attribute__ (( packed ));

/** MD5+SHA1 context size */
#define MD5_SHA1_CTX_SIZE sizeof ( struct md5_sha1_context )

/** An MD5+SHA1 digest */
struct md5_sha1_digest {
	/** MD5 digest */
	uint8_t md5[MD5_DIGEST_SIZE];
	/** SHA-1 digest */
	uint8_t sha1[SHA1_DIGEST_SIZE];
} __attribute__ (( packed ));

/** MD5+SHA1 digest size */
#define MD5_SHA1_DIGEST_SIZE sizeof ( struct md5_sha1_digest )

/** A TLS session */
struct tls_session {
	/** Reference counter */
	struct refcnt refcnt;

	/** Server name */
	const char *name;
	/** Plaintext stream */
	struct interface plainstream;
	/** Ciphertext stream */
	struct interface cipherstream;

	/** Protocol version */
	uint16_t version;
	/** Current TX cipher specification */
	struct tls_cipherspec tx_cipherspec;
	/** Next TX cipher specification */
	struct tls_cipherspec tx_cipherspec_pending;
	/** Current RX cipher specification */
	struct tls_cipherspec rx_cipherspec;
	/** Next RX cipher specification */
	struct tls_cipherspec rx_cipherspec_pending;
	/** Premaster secret */
	struct tls_pre_master_secret pre_master_secret;
	/** Master secret */
	uint8_t master_secret[48];
	/** Server random bytes */
	uint8_t server_random[32];
	/** Client random bytes */
	struct tls_client_random client_random;
	/** MD5+SHA1 context for handshake verification */
	uint8_t handshake_md5_sha1_ctx[MD5_SHA1_CTX_SIZE];
	/** SHA256 context for handshake verification */
	uint8_t handshake_sha256_ctx[SHA256_CTX_SIZE];
	/** Digest algorithm used for handshake verification */
	struct digest_algorithm *handshake_digest;
	/** Digest algorithm context used for handshake verification */
	uint8_t *handshake_ctx;
	/** Public-key algorithm used for Certificate Verify (if sent) */
	struct pubkey_algorithm *verify_pubkey;

	/** Server certificate chain */
	struct x509_chain *chain;

	/** TX sequence number */
	uint64_t tx_seq;
	/** TX pending transmissions */
	unsigned int tx_pending;
	/** TX process */
	struct process process;
	/** TX ready for plaintext data */
	int tx_ready;

	/** RX sequence number */
	uint64_t rx_seq;
	/** RX state */
	enum tls_rx_state rx_state;
	/** Offset within current RX state */
	size_t rx_rcvd;
	/** Current received record header */
	struct tls_header rx_header;
	/** Current received raw data buffer */
	void *rx_data;
};

extern int add_tls ( struct interface *xfer, const char *name,
		     struct interface **next );

#endif /* _IPXE_TLS_H */
