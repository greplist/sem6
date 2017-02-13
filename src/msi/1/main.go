package main

import (
	"bytes"
	"flag"
	"io"
	"log"
	"os"
)

const (
	perm = 438

	maxBufferSize = 512 * 1024 // 512Kб
)

var (
	fkey = flag.String("key", "secretkey", "key - secret key for encryption")
	fin  = flag.String("in", "in.txt", "fout - name of output file")
	fout = flag.String("out", "out.txt", "fout - name of output file")
)

func main() {
	flag.Parse()

	key, inName, outName := []byte(*fkey), *fin, *fout
	log.Printf(`Use "%v" as secret key`, *fkey)

	in, err := os.OpenFile(inName, os.O_RDONLY, perm)
	if err != nil {
		log.Panicln("failed open input file: err:", err)
	}
	log.Printf(`Use "%v" as input file`, inName)

	out, err := os.OpenFile(outName, os.O_CREATE|os.O_WRONLY, perm)
	if err != nil {
		log.Panicln("failed open output file: err:", err)
	}
	log.Printf(`Use "%v" as output file`, outName)

	mem := make([]byte, 0, 256)
	buffer := bytes.NewBuffer(mem)
	for nready := 0; ; buffer.Reset() {
		switch n, e := buffer.ReadFrom(io.LimitReader(in, maxBufferSize)); {
		case n == 0:
			log.Println("Encryption compliete.")
			os.Exit(0)
		case e != nil:
			log.Println("failed read block from input: err:", err)
			os.Exit(1)
		}

		buf := buffer.Bytes()
		for i := range buf {
			buf[i] ^= key[nready%len(key)]
			nready++
		}

		if _, e := buffer.WriteTo(out); e != nil {
			log.Println("failed write to output file: err:", err)
			os.Exit(1)
		}

		log.Printf("Ready %v bytes...", nready)
	}
}
