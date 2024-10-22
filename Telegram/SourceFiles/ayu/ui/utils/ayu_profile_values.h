// This is the source code of Ayu for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radscorp, 2024
#pragma once

QString IDString(not_null<PeerData*> peer);
QString IDString(MsgId topic_root_id);

rpl::producer<TextWithEntities> IDValue(not_null<PeerData*> peer);
rpl::producer<TextWithEntities> IDValue(MsgId topicRootId);
