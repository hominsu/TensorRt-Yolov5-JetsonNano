package defs

import Detect "test-yolov5-jetson-grpc/detect_service"

type Response struct {
	Status    bool       `json:"status"`
	Rect      []*BoxRect `json:"rect,omitempty"`
	Image     []byte     `json:"image,omitempty"`
}

type RespWithoutImg struct {
	Status    bool       `json:"status"`
	Rect      []*BoxRect `json:"rect,omitempty"`
}

type BoxRect struct {
	X       float64 `json:"x"`        // x 坐标
	Y       float64 `json:"y"`        // y 坐标
	Width   float64 `json:"width"`    // 宽度
	Height  float64 `json:"height"`   // 高度
	Area    float64 `json:"area"`     // 面积
	ClassId int64   `json:"class_id"` // 检测种类
}

type RpcRect struct {
	*Detect.BoxRect
}

func (r RpcRect) ToBoxRect() *BoxRect {
	return &BoxRect{
		X:       r.X,
		Y:       r.Y,
		Width:   r.Width,
		Height:  r.Height,
		Area:    r.Area,
		ClassId: r.ClassId,
	}
}

type RpcResponse struct {
	*Detect.DetectResponse
}

func (r RpcResponse) ToResponse() *Response {
	var rect []*BoxRect
	for _, boxRect := range r.Rect {
		rect = append(rect, RpcRect{boxRect}.ToBoxRect())
	}
	return &Response{
		Status:    r.Status,
		Rect:      rect,
		Image:     r.Image,
	}
}

func (r Response) WithoutImg() *RespWithoutImg {
	return &RespWithoutImg{
		Status:    r.Status,
		Rect:      r.Rect,
	}
}
