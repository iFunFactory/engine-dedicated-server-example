// Copyright (C) 2013-2016 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

////////////////////////////////////////////////////////////////////////////////
// EncryptorShift implementation.
class EncryptorShift : public Encryptor {
public:
  EncryptorShift() = default;
  virtual ~EncryptorShift() = default;

protected:
  template<typename IntegerType>
  static IntegerType CircularLeftShift(IntegerType value, size_t shift_len);
};


template<typename IntegerType>
IntegerType EncryptorShift::CircularLeftShift(IntegerType value, size_t shift_len) {
  shift_len = shift_len % (sizeof(IntegerType) * 8);
  return (value << shift_len) |
    (value >> (sizeof(IntegerType) * 8 - shift_len));
}


////////////////////////////////////////////////////////////////////////////////
// Encryptor1 implementation.
class Encryptor1 : public EncryptorShift {
public:
  Encryptor1();
  virtual ~Encryptor1();

  EncryptionType GetEncryptionType();
  static fun::string GetEncryptionName();

  void HandShake(const fun::string &key);

  bool Encrypt(fun::vector<uint8_t> &body);
  bool Decrypt(fun::vector<uint8_t> &body);

private:
  void Encrypt(size_t size, const uint8_t *src, uint8_t *dst, uint32_t *key);

  uint32_t enc_key_ = 0;
  uint32_t dec_key_ = 0;
};


Encryptor1::Encryptor1() {
}


Encryptor1::~Encryptor1() {
}


EncryptionType Encryptor1::GetEncryptionType() {
  return EncryptionType::kIFunEngine1Encryption;
}


fun::string Encryptor1::GetEncryptionName() {
  return "ife1";
}


bool Encryptor1::Encrypt(fun::vector<uint8_t> &body) {
  if (handshake_completed_ == false)
    return false;

  if (!body.empty()) {
    Encrypt(body.size(), body.data(), body.data(), &enc_key_);
  }

  return true;
}


bool Encryptor1::Decrypt(fun::vector<uint8_t> &body) {
  if (!body.empty()) {
    Encrypt(body.size(), body.data(), body.data(), &dec_key_);
  }

  return true;
}


void Encryptor1::Encrypt(size_t size, const uint8_t *src, uint8_t *dst, uint32_t *key) {
  static const size_t kBlockSize(sizeof(uint32_t));

  *key = 8253729 * *key + 2396403;

  size_t shift_len = *key & 0x0F;

  for (size_t i = 0; i < (size / kBlockSize); ++i) {
    const uint32_t *s =
    reinterpret_cast<const uint32_t *>(&src[i * kBlockSize]);
    uint32_t *d = reinterpret_cast<uint32_t *>(&dst[i * kBlockSize]);
    *d = *s ^ CircularLeftShift<uint32_t>(*key, shift_len);
  }

  for (size_t i = 0; i < (size % kBlockSize); ++i) {
    const uint8_t *s = &src[(size - 1) - i];
    uint8_t *d = &dst[(size - 1) - i];
    *d = *s ^ static_cast<uint8_t>(CircularLeftShift<uint8_t>(*key, shift_len));
  }
}


void Encryptor1::HandShake(const fun::string &key) {
  if (handshake_completed_) {
    return;
  }

  if (key.length() > 0) {
    enc_key_ = static_cast<uint32_t>(atoll(key.c_str()));
    dec_key_ = enc_key_;

    handshake_completed_ = true;
  }
}


////////////////////////////////////////////////////////////////////////////////
// Encryptor2 implementation.
class Encryptor2 : public EncryptorShift {
public:
  Encryptor2();
  virtual ~Encryptor2();

  EncryptionType GetEncryptionType();
  static fun::string GetEncryptionName();

  bool Encrypt(fun::vector<uint8_t> &body);
  bool Decrypt(fun::vector<uint8_t> &body);

private:
  void EncryptBuffer(fun::vector<uint8_t> &body, bool encrypt);
};


Encryptor2::Encryptor2() {
  handshake_completed_ = true;
}


Encryptor2::~Encryptor2() {
}


EncryptionType Encryptor2::GetEncryptionType() {
  return EncryptionType::kIFunEngine2Encryption;
}


fun::string Encryptor2::GetEncryptionName() {
  return "ife2";
}


bool Encryptor2::Encrypt(fun::vector<uint8_t> &body) {
  EncryptBuffer (body, true);
  return true;
}


bool Encryptor2::Decrypt(fun::vector<uint8_t> &body) {
  EncryptBuffer (body, false);
  return true;
}


void Encryptor2::EncryptBuffer(fun::vector<uint8_t> &body, bool encrypt) {
  uint8_t *buf = body.data();
  size_t buf_size = body.size();

  uint8_t key = static_cast<uint8_t>(buf_size & 0xFF);
  size_t shift_len = 0;
  if (encrypt) {
    shift_len = key % (sizeof(uint8_t) * 8);
  } else {
    shift_len = (sizeof(uint8_t) * 8) - (key % (sizeof(uint8_t) * 8));
  }

  for (size_t i = 0; i < buf_size; ++i) {
    if (encrypt) {
      buf[i] = buf[i] ^ key;
      buf[i] = CircularLeftShift<uint8_t>(buf[i], shift_len);
    } else {
      buf[i] = CircularLeftShift<uint8_t>(buf[i], shift_len);
      buf[i] = buf[i] ^ key;
    }
  }
}
