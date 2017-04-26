package main

import (
	"crypto/aes"
	"crypto/cipher"
	"encoding/hex"
	"time"
)

var (
	nonce []byte
)

func init() {
	nonce, _ = hex.DecodeString("37b8e8a308c354048d245f6d")
}

type Tiket struct {
	CliID      int64         `json:"client_id"`
	TimeMarker int64         `json:"time_marker"`
	TTL        time.Duration `json:"ttl"`
	Session    []byte        `json:"session"`
	ClientIP   string        `json:"client_ip"`
}

type Certificate struct {
	ServerID int64         `json:"server_id"`
	TTL      time.Duration `json:"ttl"`
	Session  []byte        `json:"session"`
}

type Request struct {
	CliID      int64 `json:"client_id"`
	ServiceID  int64 `json:"service_id"`
	TimeMarker int64 `json:"time_marker"`
}

type Response struct {
	Certificate []byte `json:"certificate"`
	Tiket       []byte `json:"tiket"`
}

func genSession() []byte {
	return []byte("fake_session")
}

func decrypt(key, ciphertext []byte) ([]byte, error) {
	ciphertext, _ := hex.DecodeString(string(ciphertext))

	block, err := aes.NewCipher(key)
	if err != nil {
		return nil, err
	}

	aesgcm, err := cipher.NewGCM(block)
	if err != nil {
		return nil, err
	}

	return aesgcm.Open(nil, nonce, ciphertext, nil)
}

func encrypt(key, plaintext []byte) ([]byte, error) {
	block, err := aes.NewCipher(key)
	if err != nil {
		return nil, err
	}

	aesgcm, err := cipher.NewGCM(block)
	if err != nil {
		return nil, err
	}

	return aesgcm.Seal(nil, nonce, plaintext, nil), nil
}
