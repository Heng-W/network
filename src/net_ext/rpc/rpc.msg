package net;

message RpcRequest
{
  ufixed64 id = 1;
  string method = 2;
  bytes content = 3;
}

message RpcResponse
{
  ufixed64 id = 1;
  int32 status = 2;
  string describe = 3;
  bytes content = 4;
}
