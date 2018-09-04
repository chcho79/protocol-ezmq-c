/*******************************************************************************
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *******************************************************************************/

#include "cezmqsubscriber.h"
#include "EZMQSubscriber.h"
#include "EZMQMessage.h"
#include "EZMQByteData.h"
#include "Event.pb.h"
#include "EZMQException.h"

using namespace ezmq;

typedef struct subscriber
{
    EZMQSubscriber *handle;
}subscriber;

void subCB(const EZMQMessage &event, csubCB subcb)
{
    if(EZMQ_CONTENT_TYPE_PROTOBUF == event.getContentType())
    {
        const Event *protoEvent;
        protoEvent =  dynamic_cast<const Event*>(&event);
        subcb((void *)protoEvent, CEZMQ_CONTENT_TYPE_PROTOBUF);
    }
    else if(EZMQ_CONTENT_TYPE_BYTEDATA == event.getContentType())
    {
        const EZMQByteData *byteData;
        byteData =  dynamic_cast<const EZMQByteData*>(&event);
        subcb((void *)byteData, CEZMQ_CONTENT_TYPE_BYTEDATA);
    }
}

void subTopicCB(std::string topic, const EZMQMessage &event, csubTopicCB topiccb)
{
    if(EZMQ_CONTENT_TYPE_PROTOBUF == event.getContentType())
    {
        const Event *protoEvent;
        protoEvent =  dynamic_cast<const Event*>(&event);
        topiccb(topic.c_str(), (void *)protoEvent, CEZMQ_CONTENT_TYPE_PROTOBUF);
    }
    else if(EZMQ_CONTENT_TYPE_BYTEDATA == event.getContentType())
    {
        const EZMQByteData *byteData;
        byteData =  dynamic_cast<const EZMQByteData*>(&event);
        topiccb(topic.c_str(), (void *)byteData, CEZMQ_CONTENT_TYPE_BYTEDATA);
    }
}

static EZMQSubscriber *getSubInstance(ezmqSubHandle_t subHandle)
{
    subscriber *subObj= static_cast<subscriber *>(subHandle);
    return subObj->handle;
}

CEZMQErrorCode ezmqCreateSubscriber(const char *ip, int port, csubCB subcb,
        csubTopicCB topiccb, ezmqSubHandle_t *subHandle)
 {
    VERIFY_NON_NULL(ip)
    if ( port < 0)
    {
        return CEZMQ_ERROR;
    }
    EZMQSubscriber *subscriberObj = nullptr ;
    subscriberObj = new(std::nothrow) EZMQSubscriber(ip, port,
                                        std::bind(subCB, std::placeholders::_1,  subcb),
                                        std::bind(subTopicCB, std::placeholders::_1, std::placeholders::_2, topiccb));

    ALLOC_ASSERT(subscriberObj)
    subscriber *subInstance = new(std::nothrow) subscriber();
    if(!subInstance)
    {
        delete subscriberObj;
        abort();
    }
    subInstance->handle =  subscriberObj;
    *subHandle = subInstance;
    return CEZMQ_OK;
 }

CEZMQErrorCode ezmqSetClientKeys(ezmqSubHandle_t subHandle, const char *clientPrivateKey,
        const char *clientPublicKey)
{
    VERIFY_NON_NULL(subHandle)
    VERIFY_NON_NULL(clientPrivateKey)
    VERIFY_NON_NULL(clientPublicKey)
    EZMQSubscriber *subscriberObj = getSubInstance(subHandle);
    EZMQErrorCode errorCode = EZMQ_ERROR;
    try
    {
        errorCode = subscriberObj->setClientKeys(clientPrivateKey, clientPublicKey);
    }
    catch(EZMQException &e)
    {
        return CEZMQErrorCode(errorCode);
    }
    return CEZMQErrorCode(errorCode);
}

CEZMQErrorCode ezmqSetServerPublicKey(ezmqSubHandle_t subHandle, const char *key)
{
    VERIFY_NON_NULL(subHandle)
    VERIFY_NON_NULL(key)
    EZMQSubscriber *subscriberObj = getSubInstance(subHandle);
    EZMQErrorCode errorCode = EZMQ_ERROR;
    try
    {
        errorCode = subscriberObj->setServerPublicKey(key);
    }
    catch(EZMQException &e)
    {
        return CEZMQErrorCode(errorCode);
    }
    return CEZMQErrorCode(errorCode);
}

CEZMQErrorCode emzqStartSubscriber(ezmqSubHandle_t subHandle)
{
    VERIFY_NON_NULL(subHandle)
    EZMQSubscriber *subscriberObj = getSubInstance(subHandle);
    return CEZMQErrorCode(subscriberObj->start());
}

 CEZMQErrorCode ezmqSubscribe(ezmqSubHandle_t subHandle)
 {
    VERIFY_NON_NULL(subHandle)
    EZMQSubscriber *subscriberObj = getSubInstance(subHandle);
    return CEZMQErrorCode(subscriberObj->subscribe());
 }

 CEZMQErrorCode ezmqSubscribeForTopic(ezmqSubHandle_t subHandle, const char *topic)
 {
    VERIFY_NON_NULL(subHandle)
    VERIFY_NON_NULL_TOPIC(topic)
    EZMQSubscriber *subscriberObj = getSubInstance(subHandle);
    return CEZMQErrorCode(subscriberObj->subscribe(topic));
 }

CEZMQErrorCode ezmqSubscribeForTopicList(ezmqSubHandle_t subHandle, const char ** topicList, int listSize)
{
    VERIFY_NON_NULL(subHandle)
    VERIFY_NON_NULL_TOPIC(topicList)
    EZMQSubscriber *subscriberObj = getSubInstance(subHandle);
    if(0 == listSize)
    {
        return CEZMQ_INVALID_TOPIC;
    }
    std::list<std::string> topics;
    for (int  i =0; i < listSize; i++)
    {
        topics.push_back(topicList[i]);
    }
    return CEZMQErrorCode(subscriberObj->subscribe(topics));
}

CEZMQErrorCode ezmqSubscribeWithIpPort(ezmqSubHandle_t subHandle, const char *ip, const int port,
        const char *topic)
{
    VERIFY_NON_NULL(subHandle)
    VERIFY_NON_NULL(ip)
    VERIFY_NON_NULL_TOPIC(topic)
    EZMQSubscriber *subscriberObj = getSubInstance(subHandle);
    return CEZMQErrorCode(subscriberObj->subscribe(ip, port, topic));
}

CEZMQErrorCode ezmqUnSubscribe(ezmqSubHandle_t subHandle)
{
    VERIFY_NON_NULL(subHandle)
    EZMQSubscriber *subscriberObj = getSubInstance(subHandle);
    return CEZMQErrorCode(subscriberObj->unSubscribe());
}

CEZMQErrorCode ezmqUnSubscribeForTopic(ezmqSubHandle_t subHandle, const char *topic)
{
    VERIFY_NON_NULL(subHandle)
    VERIFY_NON_NULL_TOPIC(topic)
    EZMQSubscriber *subscriberObj = getSubInstance(subHandle);
    return CEZMQErrorCode(subscriberObj->unSubscribe(topic));
}

CEZMQErrorCode ezmqUnSubscribeForTopicList(ezmqSubHandle_t subHandle, const char ** topicList , int listSize)
{
    VERIFY_NON_NULL(subHandle)
    VERIFY_NON_NULL_TOPIC(topicList)
    EZMQSubscriber *subscriberObj = getSubInstance(subHandle);
    if (0 == listSize)
    {
        return CEZMQ_INVALID_TOPIC;
    }
    std::list<std::string> topics;
    for (int  i =0; i < listSize; i++)
    {
        topics.push_back(topicList[i]);
    }
    return CEZMQErrorCode(subscriberObj->unSubscribe(topics));
}

CEZMQErrorCode ezmqStopSubscriber(ezmqSubHandle_t subHandle)
 {
    VERIFY_NON_NULL(subHandle)
    EZMQSubscriber *subscriberObj = getSubInstance(subHandle);
    return CEZMQErrorCode(subscriberObj->stop());
 }

CEZMQErrorCode ezmqGetSubIp(ezmqSubHandle_t subHandle, char **ip)
 {
    VERIFY_NON_NULL(subHandle)
    VERIFY_NON_NULL(ip)
    EZMQSubscriber *subscriberObj = getSubInstance(subHandle);
    *ip = (char *)subscriberObj->getIp().c_str();
    return CEZMQ_OK;
}

CEZMQErrorCode ezmqGetSubPort(ezmqSubHandle_t subHandle, int *port)
{
    VERIFY_NON_NULL(subHandle)
    VERIFY_NON_NULL(port)
    EZMQSubscriber *subscriberObj = getSubInstance(subHandle);
    *port = subscriberObj->getPort();
    return CEZMQ_OK;
}

CEZMQErrorCode ezmqDestroySubscriber(ezmqSubHandle_t *subHandle)
{
    VERIFY_NON_NULL(subHandle)
    VERIFY_NON_NULL(*subHandle)
    EZMQSubscriber *subscriberObj = getSubInstance(*subHandle);
    delete subscriberObj;
    subscriber *subObj = static_cast<subscriber *>(*subHandle);
    delete subObj;
    *subHandle = NULL;
    return CEZMQ_OK;
}
