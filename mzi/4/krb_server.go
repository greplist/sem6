package main

import (
	"encoding/json"
	"log"
	"net/http"

	"github.com/julienschmidt/httprouter"
)

var (
	clientDB = map[int64][]byte{
		1: []byte("supersecretclientpassw"),
	}
	tgsDB = map[int64][]byte{
		1: []byte("supersecrettgspassw"),
	}
	serverDB = map[int64][]byte{
		1: []byte("supersecretserverpassw"),
	}
)

func CAHandler(w http.ResponseWriter, r *http.Request, _ httprouter.Params) {
	var request Request
	if err := json.NewDecoder(w); err != nil {
		log.Println("CAHandler: json decode err:", err)
		http.Error(w, "Faled json decode", http.StatusBadRequest)
		return
	}

	cliKey, ok := clientDB[request.CliID]
	if !ok {
		log.Println("CAHandler: client does not exist:", request.CliID)
		http.Error(w, "Client does not exist", http.StatusNotFound)
		return
	}

	tgsKey, ok := tgsDB[request.ServiceID]
	if !ok {
		log.Println("CAHandler: service does not exist:", request.ServiceID)
		http.Error(w, "TGS does not exist", http.StatusNotFound)
		return
	}

	session := genSession()

}

func TGSHandler(w http.ResponseWriter, r *http.Request, _ httprouter.Params) {

}

func ServiceHandler(w http.ResponseWriter, r *http.Request, _ httprouter.Params) {

}

func main() {
	router := httprouter.New()
	router.POST("/ca", CAHandler)
	router.POST("/tgs", TGSHandler)
	router.POST("/service", ServiceHandler)

	log.Fatalln("Server HTTTP err:", http.ListenAndServe(":8080", router))
}
