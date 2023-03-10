////////////////////////////////////////////////////////////////////////////
//
// Copyright 2023 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <realm/object-store/impl/object_accessor_impl.hpp>
#include <realm/object-store/shared_realm.hpp>
#include <realm/object-store/sync/app.hpp>
#include <realm/object-store/sync/sync_manager.hpp>
#include <realm/object-store/sync/sync_session.hpp>
#include <realm/util/logger.hpp>

#include <qnx-demo/generic_network_transport.hpp>
#include <qnx-demo/test_data.hpp>

using namespace qnxdemo;

void add_person(SharedRealm& realm, CppContext& context, const Person& person)
{
    Object::create(context, realm, "Person", std::any(AnyDict{
        {"_id", ObjectId::gen()},
        {"firstName", person.firstName},
        {"lastName", person.lastName},
        {"age", static_cast<int64_t>(person.age)},
        {"realm_id", std::string("foo")}
    }));
}

int main(int argc, char** argv)
{
    std::shared_ptr<util::Logger> logger = std::make_shared<util::StderrLogger>(util::Logger::Level::debug);

    logger->info("=== QNX DEMO STARTING ===");

    Realm::Config realm_config;
    realm_config.schema_mode = SchemaMode::AdditiveExplicit;
    realm_config.schema_version = 0;
    realm_config.schema = {
        { "Person", {
            Property("_id", PropertyType::ObjectId | PropertyType::Nullable, Property::IsPrimary(true)),
            Property("firstName", PropertyType::String),
            Property("lastName", PropertyType::String),
            Property("age", PropertyType::Int),
            Property("realm_id", PropertyType::String | PropertyType::Nullable)
        }}
    };
    realm_config.path = "realm.qnxdemo";
    auto realm = Realm::get_shared_realm(realm_config);
    CppContext ctx(realm);

    auto people = new Results(realm, ObjectStore::table_for_object_type(realm->read_group(), "Person"));
    logger->info("Number of people: %1", people->size());

    auto token = people->add_notification_callback([&logger](CollectionChangeSet change) {
        logger->info("Info was empty: %1", change.empty() ? "YES" : "NO");
        logger->info("%1 person(s) inserted", change.insertions.count());
        logger->info("%1 person(s) deleted", change.deletions.count());
        logger->info("%1 person(s) modifications", change.modifications.count());
        logger->info("%1 person(s) modifications_new", change.modifications_new.count());
    });
    realm->begin_transaction();
    for (const auto& person : Person::data) {
        add_person(realm, ctx, person);
    }
    realm->commit_transaction();
    logger->info("Added one person");
    logger->info("Number of people: %1", people->size());

    return 0;
}