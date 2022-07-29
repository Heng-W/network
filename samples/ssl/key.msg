package net;

message ServerRSAKey {
  string key = 1;
}

message EncryptedAESKey {
  string key = 1;
}

message Text {
  string what = 1;
}