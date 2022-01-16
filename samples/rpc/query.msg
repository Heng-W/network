package test;

message Query {
  int64 id = 1;
  string questioner = 2;

	repeated int16 a = 4;
  repeated string question = 3;
}

message Answer {
  int64 id = 1;
  string questioner = 2;
  string answerer = 3;

  repeated string solution = 4;
  
  message Test {
	  int32 id = 1;
  }
}

message Empty {
  int32 id = 1;
}
