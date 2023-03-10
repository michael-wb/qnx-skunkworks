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
        {"age", person.age},
        {"realm_id", std::string("qnx-demo")}
    }));
}

void usage(const std::shared_ptr<util::Logger>& logger)
{
    logger->info("usage: qnxdemo [-h] [command]");
    logger->info("QNX Demo - display and add data to the realm database");
    logger->info("Command:");
    logger->info(" add1  - add the first set of people to the database");
    logger->info(" add2  - add the second set of people to the database");
    logger->info(" add3  - add the third set of people to the database");
    logger->info("");
}

int main(int argc, char** argv)
{
    std::shared_ptr<util::Logger> logger = std::make_shared<util::StderrLogger>(util::Logger::Level::debug);

    if (argc > 1 && std::string(argv[1]) == "-h") {
        usage(logger);
        exit(0);
    }

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

    if (argc > 1 && std::string(argv[1]).find("add") != std::string::npos) {
        auto realm = Realm::get_shared_realm(realm_config);
        CppContext ctx(realm);

        auto& personData = [&]() -> const std::vector<Person>& {
            if (std::string(argv[1]).find("add1") != std::string::npos) {
                logger->info("\n=== QNX DEMO: ADDING PEOPLE (SET 1) TO DATABASE ===");
                return Person::data1;
            }
            else if (std::string(argv[1]).find("add2") != std::string::npos) {
                logger->info("\n=== QNX DEMO: ADDING PEOPLE (SET 2) TO DATABASE ===");
                return Person::data2;
            }
            else if (std::string(argv[1]).find("add3") != std::string::npos) {
                logger->info("\n=== QNX DEMO: ADDING PEOPLE (SET 3) TO DATABASE ===");
                return Person::data3;
            }
            else {
                logger->error("invalid 'add' argument - use '-h' for help");
                exit(1);
            }
        }();

        realm->begin_transaction();
        for (const auto& person : personData) {
            add_person(realm, ctx, person);
            logger->info("Added person: %1 %2 (age %3)", person.firstName, person.lastName, person.age);
        }
        realm->commit_transaction();
    }

    auto realm = Realm::get_shared_realm(realm_config);
    CppContext ctx(realm);

    auto people = new Results(realm, ObjectStore::table_for_object_type(realm->read_group(), "Person"));
    logger->info("\n=== QNX DEMO: DATABASE CONTENTS ===");
    logger->info("Number of people in realm: %1", people->size());
    for (int i = 0; i < people->size(); i++) {
        auto person = people->get(i);
        logger->info("Found person: %1 %2 (age %3)", person.get<String>("firstName"), person.get<String>("lastName"), person.get<Int>("age"));
    }
    logger->info("");

    return 0;
}