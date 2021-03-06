// Code generated by protoc-gen-go-grpc. DO NOT EDIT.

package Detect

import (
	context "context"
	grpc "google.golang.org/grpc"
	codes "google.golang.org/grpc/codes"
	status "google.golang.org/grpc/status"
)

// This is a compile-time assertion to ensure that this generated file
// is compatible with the grpc package it is being compiled against.
// Requires gRPC-Go v1.32.0 or later.
const _ = grpc.SupportPackageIsVersion7

// DetectResultServiceClient is the client API for DetectResultService service.
//
// For semantics around ctx use and closing/ending streaming RPCs, please refer to https://pkg.go.dev/google.golang.org/grpc/?tab=doc#ClientConn.NewStream.
type DetectResultServiceClient interface {
	DetectedRect(ctx context.Context, in *DetectRequest, opts ...grpc.CallOption) (*DetectResponse, error)
}

type detectResultServiceClient struct {
	cc grpc.ClientConnInterface
}

func NewDetectResultServiceClient(cc grpc.ClientConnInterface) DetectResultServiceClient {
	return &detectResultServiceClient{cc}
}

func (c *detectResultServiceClient) DetectedRect(ctx context.Context, in *DetectRequest, opts ...grpc.CallOption) (*DetectResponse, error) {
	out := new(DetectResponse)
	err := c.cc.Invoke(ctx, "/Detect.DetectResultService/DetectedRect", in, out, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

// DetectResultServiceServer is the server API for DetectResultService service.
// All implementations must embed UnimplementedDetectResultServiceServer
// for forward compatibility
type DetectResultServiceServer interface {
	DetectedRect(context.Context, *DetectRequest) (*DetectResponse, error)
	mustEmbedUnimplementedDetectResultServiceServer()
}

// UnimplementedDetectResultServiceServer must be embedded to have forward compatible implementations.
type UnimplementedDetectResultServiceServer struct {
}

func (UnimplementedDetectResultServiceServer) DetectedRect(context.Context, *DetectRequest) (*DetectResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "method DetectedRect not implemented")
}
func (UnimplementedDetectResultServiceServer) mustEmbedUnimplementedDetectResultServiceServer() {}

// UnsafeDetectResultServiceServer may be embedded to opt out of forward compatibility for this service.
// Use of this interface is not recommended, as added methods to DetectResultServiceServer will
// result in compilation errors.
type UnsafeDetectResultServiceServer interface {
	mustEmbedUnimplementedDetectResultServiceServer()
}

func RegisterDetectResultServiceServer(s grpc.ServiceRegistrar, srv DetectResultServiceServer) {
	s.RegisterService(&DetectResultService_ServiceDesc, srv)
}

func _DetectResultService_DetectedRect_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(DetectRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(DetectResultServiceServer).DetectedRect(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/Detect.DetectResultService/DetectedRect",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(DetectResultServiceServer).DetectedRect(ctx, req.(*DetectRequest))
	}
	return interceptor(ctx, in, info, handler)
}

// DetectResultService_ServiceDesc is the grpc.ServiceDesc for DetectResultService service.
// It's only intended for direct use with grpc.RegisterService,
// and not to be introspected or modified (even as a copy)
var DetectResultService_ServiceDesc = grpc.ServiceDesc{
	ServiceName: "Detect.DetectResultService",
	HandlerType: (*DetectResultServiceServer)(nil),
	Methods: []grpc.MethodDesc{
		{
			MethodName: "DetectedRect",
			Handler:    _DetectResultService_DetectedRect_Handler,
		},
	},
	Streams:  []grpc.StreamDesc{},
	Metadata: "detect.proto",
}
