/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef AVROPROTOCOLPACKET_HH_3414660241__H_
#define AVROPROTOCOLPACKET_HH_3414660241__H_


#include <sstream>
#include "boost/any.hpp"
#include "avro/Specific.hh"
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"

namespace c {
struct Sample1 {
    float speed;
    float move;
    Sample1() :
        speed(float()),
        move(float())
        { }
};

}
namespace avro {
template<> struct codec_traits<c::Sample1> {
    static void encode(Encoder& e, const c::Sample1& v) {
        avro::encode(e, v.speed);
        avro::encode(e, v.move);
    }
    static void decode(Decoder& d, c::Sample1& v) {
        avro::decode(d, v.speed);
        avro::decode(d, v.move);
    }
};

}
#endif
