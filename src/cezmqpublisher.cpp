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

#include "cezmqpublisher.h"
#include "EZMQPublisher.h"
#include "Event.pb.h"
#include "EZMQByteData.h"
#include "EZMQException.h"

using namespace ezmq;

typedef struct publisher
{
    EZMQPublisher *handle;
} publisher;

void startCallback(EZMQErrorCode /*code*/, ezmqStartCB /*startCb*/){}
void stopCallback(EZMQErrorCode /*code*/, ezmqStopCB /*stopCb*/){}
void errorCalback(EZMQErrorCode /*code*/, ezmqErrorCB /*errorCb*/){}

static EZMQPublisher *getPubInstance(ezmqPubHandle_t pubHandle)
{
    publisher *pubObj = static_cast<publisher *>(pubHandle);
    return pubObj->handle;
}

CEZMQErrorCode ezmqCreatePublisher(int port, ezmqStartCB startCb,
        ezmqStopCB stopCb, ezmqErrorCB errorCb, ezmqPubHandle_t *pubHandle)
{
    if (port < 0)
    {
        return CEZMQ_ERROR;
    }
    EZMQPublisher *publisherObj = nullptr ;
    publisherObj =  new(std::nothrow) EZMQPublisher(port,  std::bind(startCallback, std::placeholders::_1, startCb),
                                                                               std::bind(stopCallback,  std::placeholders::_1, stopCb),
                                                                               std::bind(errorCalback,  std::placeholders::_1, errorCb));
    ALLOC_ASSERT(publisherObj)
    publisher *pubInstance = new(std::nothrow) publisher();
    if(!pubInstance)
    {
        delete publisherObj;
        abort();
    }
    pubInstance->handle = publisherObj;
    *pubHandle = pubInstance;
    return CEZMQ_OK;
}

CEZMQErrorCode ezmqSetServerPrivateKey(ezmqPubHandle_t pubHandle, const char *key)
{
    VERIFY_NON_NULL(pubHandle)
    VERIFY_NON_NULL(key)
    EZMQPublisher *publisherObj = getPubInstance(pubHandle);
    EZMQErrorCode errorCode = EZMQ_ERROR;
    try
    {
        errorCode = publisherObj->setServerPrivateKey(key);
    }
    catch(EZMQException &e)
    {
        return CEZMQErrorCode(errorCode);
    }
    return CEZMQErrorCode(errorCode);
}

CEZMQErrorCode ezmqStartPublisher(ezmqPubHandle_t pubHandle)
{
    VERIFY_NON_NULL(pubHandle)
    EZMQPublisher *publisherObj = getPubInstance(pubHandle);
    return CEZMQErrorCode(publisherObj->start());
}

CEZMQErrorCode ezmqPublish(ezmqPubHandle_t pubHandle, const ezmqMsgHandle_t event)
{
    VERIFY_NON_NULL(pubHandle)
    VERIFY_NON_NULL(event)
    EZMQPublisher *publisherObj = getPubInstance(pubHandle);

    const ezmq::EZMQMessage *ezmqMessage = static_cast<const ezmq::EZMQMessage *>(event);
    if(EZMQ_CONTENT_TYPE_PROTOBUF == ezmqMessage->getContentType())
    {
        return CEZMQErrorCode(publisherObj->publish(*(static_cast<const ezmq::Event *>(event))));
    }
    else if(EZMQ_CONTENT_TYPE_BYTEDATA == ezmqMessage->getContentType())
    {
        return CEZMQErrorCode(publisherObj->publish(*(static_cast<const ezmq::EZMQByteData *>(event))));
    }
    else
    {
        return CEZMQ_INVALID_CONTENT_TYPE;
    }
}

CEZMQErrorCode ezmqPublishOnTopic(ezmqPubHandle_t pubHandle, const char *topic, const ezmqMsgHandle_t event)
{
    VERIFY_NON_NULL(pubHandle)
    VERIFY_NON_NULL(event)
    VERIFY_NON_NULL_TOPIC(topic)
    EZMQPublisher *publisherObj = getPubInstance(pubHandle);

    const ezmq::EZMQMessage *ezmqMessage = static_cast<const ezmq::EZMQMessage *>(event);
    if(EZMQ_CONTENT_TYPE_PROTOBUF == ezmqMessage->getContentType())
    {
        return CEZMQErrorCode(publisherObj->publish(topic, *(static_cast<const ezmq::Event *>(event))));
    }
    else if(EZMQ_CONTENT_TYPE_BYTEDATA == ezmqMessage->getContentType())
    {
        return CEZMQErrorCode(publisherObj->publish(topic, *(static_cast<const ezmq::EZMQByteData *>(event))));
    }
    else
    {
        return CEZMQ_INVALID_CONTENT_TYPE;
    }
}

CEZMQErrorCode ezmqPublishOnTopicList(ezmqPubHandle_t pubHandle, const char ** topicList,
        int listSize, const ezmqMsgHandle_t event)
{
    VERIFY_NON_NULL(pubHandle)
    VERIFY_NON_NULL(event)
    VERIFY_NON_NULL_TOPIC(topicList)
    EZMQPublisher *publisherObj = getPubInstance(pubHandle);
    if (0 == listSize)
    {
        return CEZMQ_INVALID_TOPIC;
    }
    std::list<std::string> topics;
    for (int  i =0; i < listSize; i++)
    {
        topics.push_back(topicList[i]);
    }

    const ezmq::EZMQMessage *ezmqMessage = static_cast<const ezmq::EZMQMessage *>(event);
    if(EZMQ_CONTENT_TYPE_PROTOBUF == ezmqMessage->getContentType())
    {
        return CEZMQErrorCode(publisherObj->publish(topics, *(static_cast<const ezmq::Event *>(event))));
    }
    else if(EZMQ_CONTENT_TYPE_BYTEDATA == ezmqMessage->getContentType())
    {
        return CEZMQErrorCode(publisherObj->publish(topics, *(static_cast<const ezmq::EZMQByteData *>(event))));
    }
    else
    {
        return CEZMQ_INVALID_CONTENT_TYPE;
    }
}

CEZMQErrorCode ezmqStopPublisher(ezmqPubHandle_t pubHandle)
{
    VERIFY_NON_NULL(pubHandle)
    EZMQPublisher *publisherObj = getPubInstance(pubHandle);
    return CEZMQErrorCode(publisherObj->stop());
}

CEZMQErrorCode ezmqGetPubPort(ezmqPubHandle_t pubHandle,  int *port)
{
    VERIFY_NON_NULL(pubHandle)
    VERIFY_NON_NULL(port)
    EZMQPublisher *publisherObj = getPubInstance(pubHandle);
    *port = publisherObj->getPort();
    return CEZMQ_OK;
}

CEZMQErrorCode ezmqDestroyPublisher(ezmqPubHandle_t *pubHandle)
{
    VERIFY_NON_NULL(pubHandle)
    VERIFY_NON_NULL(*pubHandle)
    EZMQPublisher *publisherObj = getPubInstance(*pubHandle);
    delete publisherObj;
    publisher *pubObj = static_cast<publisher *>(*pubHandle);
    delete pubObj;
    *pubHandle = NULL;
    return CEZMQ_OK;
}

