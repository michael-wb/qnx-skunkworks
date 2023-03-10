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
#include <unistd.h>

#include <realm/object-store/impl/object_accessor_impl.hpp>
#include <realm/object-store/shared_realm.hpp>
#include <realm/object-store/sync/app.hpp>
#include <realm/object-store/sync/sync_manager.hpp>
#include <realm/object-store/sync/sync_session.hpp>
#include <realm/util/logger.hpp>

#include <qnx-demo/generic_network_transport.hpp>
#include <qnx-demo/test_data.hpp>

using namespace qnxdemo;

void usage(const std::shared_ptr<util::Logger> &logger)
{
    logger->info("usage: qnxdemo [-h] [command]");
    logger->info("QNX Demo - display and add data to the realm database");
    logger->info("Command:");
    logger->info(" add1        - add the first set of people to the database");
    logger->info(" add2        - add the second set of people to the database");
    logger->info(" add3        - add the third set of people to the database");
    logger->info(" createuser  - create a user in the cloud app");
    logger->info("");
}

auto create_app_config()
{
    app::App::Config app_config;
    app_config.app_id = "qnx-demo-dqrxv";
    app_config.device_info.sdk = "object-store";
    app_config.device_info.sdk_version = "dev";
    app_config.device_info.platform = "qnx";
    app_config.device_info.platform_version = "7.1";
    app_config.transport = std::make_shared<DefaultTransport>();
    return app_config;
}

auto create_sync_config(std::string path,
                        const std::shared_ptr<util::Logger> &logger)
{
    SyncClientConfig sync_config;
    sync_config.metadata_mode = SyncClientConfig::MetadataMode::NoEncryption;
    sync_config.base_file_path = path;
    sync_config.logger_factory =
        [&](util::Logger::Level) -> std::shared_ptr<util::Logger>
    {
        return logger;
    };
    sync_config.log_level = util::Logger::Level::debug;
    return sync_config;
}

auto create_realm_config(
    std::string path, const std::shared_ptr<util::Logger> &logger,
    const std::shared_ptr<app::App> &app = nullptr,
    const std::shared_ptr<realm::SyncUser> &user = nullptr)
{
    Realm::Config realm_config;
    realm_config.schema_mode = SchemaMode::AdditiveExplicit;
    realm_config.schema_version = 0;
    realm_config.schema = {
        {"Person",
         {Property("_id", PropertyType::ObjectId, Property::IsPrimary(true)),
          Property("firstName", PropertyType::String | PropertyType::Nullable),
          Property("lastName", PropertyType::String | PropertyType::Nullable),
          Property("age", PropertyType::Int | PropertyType::Nullable),
          Property("realm_id", PropertyType::String)}}};
    if (app && user)
    {
        realm_config.sync_config =
            std::make_shared<SyncConfig>(user, SyncConfig::FLXSyncEnabled{});
        realm_config.path =
            app->sync_manager()->path_for_realm(*realm_config.sync_config);
    }
    else
    {
        realm_config.path = path + "/realm.qnxdemo";
    }
    logger->info("Using realm path: %1", realm_config.path);
    return realm_config;
}

void add_person(const SharedRealm &realm, CppContext &context,
                const Person &person)
{
    Object::create(context, realm, "Person",
                   std::any(AnyDict{{"_id", ObjectId::gen()},
                                    {"firstName", person.firstName},
                                    {"lastName", person.lastName},
                                    {"age", person.age},
                                    {"realm_id", std::string("qnx-demo")}}));
}

void add_realm_data(std::string command, const SharedRealm &realm,
                    const std::shared_ptr<util::Logger> &logger)
{
    auto &personData = [&]() -> const std::vector<Person> &
    {
        if (command == "add1")
        {
            logger->info("\n=== QNX DEMO: ADDING PEOPLE (SET 1) TO DATABASE ===");
            return Person::data1;
        }
        else if (command == "add2")
        {
            logger->info("\n=== QNX DEMO: ADDING PEOPLE (SET 2) TO DATABASE ===");
            return Person::data2;
        }
        else if (command == "add3")
        {
            logger->info("\n=== QNX DEMO: ADDING PEOPLE (SET 3) TO DATABASE ===");
            return Person::data3;
        }
        else
        {
            logger->error("invalid 'add' command - use '-h' for help");
            exit(1);
        }
    }();
    CppContext ctx(realm);
    realm->begin_transaction();
    for (const auto &person : personData)
    {
        add_person(realm, ctx, person);
        logger->info("Added person: %1 %2 (age %3)", person.firstName,
                     person.lastName, person.age);
    }
    realm->commit_transaction();
}

void read_realm_data(const SharedRealm &realm,
                     const std::shared_ptr<util::Logger> &logger)
{
    auto people = new Results(
        realm, ObjectStore::table_for_object_type(realm->read_group(), "Person"));
    logger->info("\n=== QNX DEMO: DATABASE CONTENTS ===");
    logger->info("Number of people in realm: %1", people->size());
    for (int i = 0; i < people->size(); i++)
    {
        auto person = people->get(i);
        logger->info("Found person: %1 %2 (age %3)",
                     person.get<String>("firstName"),
                     person.get<String>("lastName"), person.get<Int>("age"));
    }
    logger->info("");
}

void wait_for_upload_completion(const std::shared_ptr<SyncSession> &session,
                                const std::shared_ptr<util::Logger> &logger)
{
    auto [u_promise, u_future] = util::make_promise_future<void>();
    session->wait_for_upload_completion(
        [promise = std::move(u_promise), &logger](std::error_code error) mutable
        {
            if (error)
            {
                logger->error("failed to download: %1", error.message());
                exit(1);
            }
            promise.emplace_value();
        });
    u_future.get();
}

void wait_for_download_completion(const std::shared_ptr<SyncSession> &session,
                                  const std::shared_ptr<util::Logger> &logger)
{
    auto [d_promise, d_future] = util::make_promise_future<void>();
    session->wait_for_download_completion(
        [promise = std::move(d_promise), &logger](std::error_code error) mutable
        {
            if (error)
            {
                logger->error("failed to download: %1", error.message());
                exit(1);
            }
            promise.emplace_value();
        });
    d_future.get();
}

int main(int argc, char **argv)
{
    std::string email = "someone@qnxdemo.com";
    std::string password = "aewfl;kj98eafelkj";
    char buff[256];
    auto path = std::string(getcwd(buff, 256));
    std::shared_ptr<util::Logger> logger =
        std::make_shared<util::StderrLogger>(util::Logger::Level::info);
    app::App::Config app_config;
    SyncClientConfig sync_config;
    app::SharedApp app = nullptr;

    if (argc > 1 && std::string(argv[1]) == "-h")
    {
        usage(logger);
        exit(0);
    }
    if (argc > 1 && std::string(argv[1]) == "createuser")
    {
        app_config = create_app_config();
        sync_config = create_sync_config(path, logger);
        app = app::App::get_shared_app(app_config, sync_config);
        app->provider_client<app::App::UsernamePasswordProviderClient>()
            .register_email(
                email, password, [&](util::Optional<app::AppError> error)
                {
              if (error) {
                logger->error("Failed to create user: %1", error->message);
                exit(1);
              } });
        app->log_in_with_credentials(
            realm::app::AppCredentials::username_password(email, password),
            [&](std::shared_ptr<realm::SyncUser> user,
                util::Optional<app::AppError> error)
            {
                if (error || !user)
                {
                    logger->error("Failed to log in user: %1", error->message);
                    exit(1);
                }
                logger->info("Created user: %1", email);
            });
        return 0;
    }
    if ((argc > 1 && std::string(argv[1]) == "sync") ||
        (argc > 2 && std::string(argv[2]) == "sync"))
    {
        app_config = create_app_config();
        sync_config = create_sync_config(path, logger);
        app = app::App::get_shared_app(app_config, sync_config);
        app->log_in_with_credentials(
            realm::app::AppCredentials::username_password(email, password),
            [&](std::shared_ptr<realm::SyncUser> user,
                util::Optional<app::AppError> error)
            {
                if (error || !user)
                {
                    logger->error("Failed to log in user: %1", error->message);
                    exit(1);
                }
            });
        auto realm_config =
            create_realm_config(path, logger, app, app->current_user());
        auto realm = Realm::get_shared_realm(realm_config);
        auto session = app->sync_manager()->get_existing_session(realm_config.path);
        if (!session)
        {
            logger->error("session is null");
            exit(1);
        }
        logger->info("Session opened");
        wait_for_download_completion(session, logger);
        {
            auto sub_set = session->get_flx_subscription_store()->get_latest();
            auto mut_subs = sub_set.make_mutable_copy();
            auto realm = Realm::get_shared_realm(realm_config);
            auto table = realm->read_group().get_table(
                ObjectStore::table_name_for_object_type("Person"));
            auto col_key = table->get_column_key("realm_id");
            auto query = Query(table);
            auto flx_sub = mut_subs.find(query);
            if (!flx_sub)
            {
                logger->info("Adding flx subscription: %1", query.get_description());
                mut_subs.insert_or_assign(query);
                mut_subs.commit();
                auto subs_future = mut_subs.get_state_change_notification(
                    sync::SubscriptionSet::State::Complete);
                subs_future.get();
            }
            else
            {
                logger->info("Existing flx subscription: %1", flx_sub->query_string);
            }
            auto [s_promise, s_future] = util::make_promise_future<void>();
            wait_for_upload_completion(session, logger);
            wait_for_download_completion(session, logger);
        }

        if (argc > 1 && std::string(argv[1]).find("add") != std::string::npos)
        {
            auto realm = Realm::get_shared_realm(realm_config);
            auto session =
                app->sync_manager()->get_existing_session(realm_config.path);
            if (!session)
            {
                logger->error("session is null");
            }
            wait_for_upload_completion(session, logger);
            wait_for_download_completion(session, logger);
            add_realm_data(argv[1], realm, logger);
            wait_for_upload_completion(session, logger);
            wait_for_download_completion(session, logger);
        }
        if (argc > 1 && std::string(argv[1]) == "wait")
        {
            while (true)
            {
                auto realm = Realm::get_shared_realm(realm_config);
                auto session =
                    app->sync_manager()->get_existing_session(realm_config.path);
                if (!session)
                {
                    logger->error("session is null");
                }
                wait_for_upload_completion(session, logger);
                wait_for_download_completion(session, logger);
                read_realm_data(realm, logger);
                sleep(2);
            }
            return 1;
        }
        else if (argc > 2)
        {
            logger->error("invalid '%1' command - use '-h' for help", argv[1]);
            exit(1);
        }
        {
            auto realm = Realm::get_shared_realm(realm_config);
            auto session =
                app->sync_manager()->get_existing_session(realm_config.path);
            if (!session)
            {
                logger->error("session is null");
            }
            wait_for_upload_completion(session, logger);
            wait_for_download_completion(session, logger);
            read_realm_data(realm, logger);
        }

        return 0;
    }

    auto realm_config = create_realm_config(path, logger);
    auto realm = Realm::get_shared_realm(realm_config);

    if (argc > 1 && std::string(argv[1]).find("add") != std::string::npos)
    {

        CppContext ctx(realm);

        add_realm_data(argv[1], realm, logger);
    }
    else if (argc > 1)
    {
        logger->error("invalid '%1' command - use '-h' for help", argv[1]);
        exit(1);
    }
    read_realm_data(realm, logger);

    return 0;
}
