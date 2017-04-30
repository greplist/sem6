package main

import (
	"bytes"
	"crypto/sha1"
	"encoding/binary"
	"flag"
	"fmt"
	"io/ioutil"
)

var (
	s string
	f string
)

func init() {
	flag.StringVar(&s, "k", "testkey", "k - key for sha1")
	flag.StringVar(&f, "f", "sha1.go", "f - file for imit")
}

func makeBlocks(data []byte) [][16]uint32 {
	size := uint64(len(data))

	// add 1 as bit & some 0-bits
	var tmp [64]byte
	tmp[0] = 0x80
	if size%64 < 56 {
		data = append(data, tmp[0:56-size%64]...)
	} else {
		data = append(data, tmp[0:64+56-size%64]...)
	}

	// put size
	size <<= 3
	for i := uint(0); i < 8; i++ {
		tmp[i] = byte(size >> (56 - 8*i))
	}
	data = append(data, tmp[0:8]...)

	// repack bits to blocks
	nblocks := len(data) / 64
	blocks := make([][16]uint32, 0, nblocks)
	for i := 0; i < nblocks; i++ {
		block := [16]uint32{}
		for i := range block {
			block[i] = binary.BigEndian.Uint32(data[4*i : 4*(i+1)])
		}
		data = data[4*16:]

		blocks = append(blocks, block)
	}

	return blocks
}

func leftrotate(a, offset uint32) uint32 {
	return (a << offset) | (a >> (32 - offset))
}

func mysha1(data []byte) (hash []byte) {
	blocks := makeBlocks(data)

	var h0, h1, h2, h3, h4 uint32
	h0, h1, h2, h3, h4 = 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0
	for i := range blocks {
		w := make([]uint32, 80)
		copy(w, blocks[i][:])
		for i := 16; i < 80; i++ {
			w[i] = leftrotate(w[i-3]^w[i-8]^w[i-14]^w[i-16], 1)
		}

		a, b, c, d, e := h0, h1, h2, h3, h4

		for i := range w {
			var f, k uint32
			switch {
			case i < 20:
				f = (b & c) | ((^b) & d)
				k = 0x5A827999
			case (20 <= i) && (i < 40):
				f = b ^ c ^ d
				k = 0x6ED9EBA1
			case (40 <= i) && (i < 60):
				f = (b & c) | (b & d) | (c & d)
				k = 0x8F1BBCDC
			case (60 <= i) && (i < 80):
				f = b ^ c ^ d
				k = 0xCA62C1D6
			}

			temp := leftrotate(a, 5) + f + e + k + w[i]
			e = d
			d = c
			c = leftrotate(b, 30)
			b = a
			a = temp
		}

		h0, h1, h2, h3, h4 = h0+a, h1+b, h2+c, h3+d, h4+e
	}

	endian := binary.BigEndian
	buf := bytes.NewBuffer(make([]byte, 0, 20))
	binary.Write(buf, endian, h0)
	binary.Write(buf, endian, h1)
	binary.Write(buf, endian, h2)
	binary.Write(buf, endian, h3)
	binary.Write(buf, endian, h4)
	return buf.Bytes()
}

func main() {
	flag.Parse()

	fmt.Println("Start with key:", s)
	data := []byte(s)

	fmt.Printf("Sha1 custom: % x\n", mysha1(data))
	fmt.Printf("Sha1 stdlib: % x\n", sha1.Sum(data))

	data, _ = ioutil.ReadFile(f)
	fmt.Printf("Imit custom: %x \n", mysha1(data))
	fmt.Printf("Imit stdlib: %x\n", sha1.Sum(data))
}
