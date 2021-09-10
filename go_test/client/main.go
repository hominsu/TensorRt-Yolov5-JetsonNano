package main

import (
	"context"
	"encoding/json"
	"fmt"
	"gocv.io/x/gocv"
	"google.golang.org/grpc"
	"log"
	"test-yolov5-jetson-grpc/client/defs"
	Detect "test-yolov5-jetson-grpc/detect_service"
	"time"
)

func main() {
	conn, err := grpc.Dial("192.168.1.114:50005", grpc.WithTransportCredentials(Detect.GetClientCreds()))
	if err != nil {
		log.Panicln(err.Error())
	}
	detectResultServiceClient := Detect.NewDetectResultServiceClient(conn)

	ctx := context.Background()

	window := gocv.NewWindow("image")

	for {
		resp, err := detectResultServiceClient.DetectedRect(ctx, &Detect.DetectRequest{Status: true})
		if err != nil {
			log.Println(err.Error())
			continue
		}

		r := defs.RpcResponse{DetectResponse: resp}.ToResponse()
		bytes, err := json.Marshal(r.WithoutImg())
		if err != nil {
			log.Println("WithoutImg: ", err.Error())
			return
		}
		fmt.Println(string(bytes))

		if r.Status {
			if decodeImg, err := gocv.IMDecode(r.Image, gocv.IMReadColor); err != nil {
				log.Println("IMDecode: ",err.Error())
				return
			} else {
				//gocv.IMWrite(time.Now().String()+".jpg", decodeImg)
				window.IMShow(decodeImg)
				window.WaitKey(1)
			}
		}
		time.Sleep(time.Millisecond * 39)
	}
}
