# neonsignal binary
srcs = [
  'main.c++',
  # api_handler
  'neonsignal/api_handler.c++',
  'neonsignal/api_handler/auth_login_finish_headers.c++',
  'neonsignal/api_handler/auth_login_options.c++',
  'neonsignal/api_handler/auth_user_check.c++',
  'neonsignal/api_handler/user_register.c++',
  'neonsignal/api_handler/user_verify.c++',
  'neonsignal/api_handler/user_enroll.c++',
  'neonsignal/api_handler/codex_brief.c++',
  'neonsignal/api_handler/codex_image.c++',
  'neonsignal/api_handler/codex_item.c++',
  'neonsignal/api_handler/codex_list.c++',
  'neonsignal/api_handler/codex_run_artifacts.c++',
  'neonsignal/api_handler/codex_run_start.c++',
  'neonsignal/api_handler/codex_run_status.c++',
  'neonsignal/api_handler/codex_run_stderr.c++',
  'neonsignal/api_handler/codex_run_stdout.c++',
  'neonsignal/api_handler/cpu_stream.c++',
  'neonsignal/api_handler/events.c++',
  'neonsignal/api_handler/identify_api_route.c++',
  'neonsignal/api_handler/incoming_data.c++',
  'neonsignal/api_handler/memory_stream.c++',
  'neonsignal/api_handler/redirect_service_stream.c++',
  'neonsignal/api_handler/stats.c++',
  # event_loop
  'neonsignal/event_loop.c++',
  'neonsignal/event_loop/add_fd.c++',
  'neonsignal/event_loop/add_timer.c++',
  'neonsignal/event_loop/remove_fd.c++',
  'neonsignal/event_loop/run.c++',
  'neonsignal/event_loop/shutdown_graceful.c++',
  'neonsignal/event_loop/stop.c++',
  'neonsignal/event_loop/update_fd.c++',
  # hpack_decoder
  'neonsignal/hpack_decoder.c++',
  'neonsignal/hpack_decoder/decode.c++',
  # http2_listener
  'neonsignal/http2_listener.c++',
  'neonsignal/http2_listener/close_connection_.c++',
  'neonsignal/http2_listener/handle_accept_.c++',
  'neonsignal/http2_listener/handle_connection_.c++',
  'neonsignal/http2_listener/handle_io_.c++',
  'neonsignal/http2_listener/helper/build_frame.c++',
  'neonsignal/http2_listener/helper/build_response_frames.c++',
  'neonsignal/http2_listener/helper/build_server_settings.c++',
  'neonsignal/http2_listener/helper/build_settings_ack.c++',
  'neonsignal/http2_listener/helper/build_window_update.c++',
  'neonsignal/http2_listener/helper/decode_integer.c++',
  'neonsignal/http2_listener/helper/encode_integer.c++',
  'neonsignal/http2_listener/helper/encode_literal_header_no_index.c++',
  'neonsignal/http2_listener/helper/encode_string.c++',
  'neonsignal/http2_listener/helper/guess_content_type.c++',
  'neonsignal/http2_listener/helper/load_static.c++',
  'neonsignal/http2_listener/helper/make_listen_socket.c++',
  'neonsignal/http2_listener/helper/read_rss_kb.c++',
  'neonsignal/http2_listener/probe_redirect_service_.c++',
  'neonsignal/http2_listener/register_connection_.c++',
  'neonsignal/http2_listener/setup_listener_.c++',
  'neonsignal/http2_listener/start.c++',
  'neonsignal/http2_listener/start_redirect_monitor_.c++',
  'neonsignal/http2_listener/stop_redirect_monitor_.c++',
  # logging
  'neonsignal/logging.c++',
  # neonsignal
  'neonsignal/neonsignal.c++',
  # router
  'neonsignal/router.c++',
  'neonsignal/router/resolve.c++',
  # server
  'neonsignal/server/configure_credentials.c++',
  'neonsignal/server/initialize_tls.c++',
  'neonsignal/server/run.c++',
  'neonsignal/server/ssl_context_deleter_operator.c++',
  'neonsignal/server/stop.c++',
  # thread pool
  'neonsignal/thread_pool.c++',
  'neonsignal/thread_pool/enqueue.c++',
  'neonsignal/thread_pool/worker_.c++',
  # vhost
  'neonsignal/vhost.c++',
  'neonsignal/vhost/resolve.c++',
  # cert_manager
  'neonsignal/cert_manager.c++',
  'neonsignal/cert_manager/initialize.c++',
  'neonsignal/cert_manager/get_context.c++',
  # codex_runner
  'neonsignal/codex_runner.c++',
  'neonsignal/codex_runner/build_prompt_.c++',
  'neonsignal/codex_runner/get_config_.c++',
  'neonsignal/codex_runner/get_config_bool_.c++',
  'neonsignal/codex_runner/get_config_u64_.c++',
  'neonsignal/codex_runner/run_async_.c++',
  'neonsignal/codex_runner/sanitize_filename_.c++',
  'neonsignal/codex_runner/start_run.c++',
  # database
  'neonsignal/database/database.c++',
  'neonsignal/database/serialization.c++',
  # webauthn
  'neonsignal/webauthn.c++',
]

# redirector binary
redirect_srcs = [
  # event_loop
  'neonsignal/event_loop.c++',
  'neonsignal/event_loop/add_fd.c++',
  'neonsignal/event_loop/add_timer.c++',
  'neonsignal/event_loop/remove_fd.c++',
  'neonsignal/event_loop/run.c++',
  'neonsignal/event_loop/shutdown_graceful.c++',
  'neonsignal/event_loop/stop.c++',
  'neonsignal/event_loop/update_fd.c++',
  # logging
  'neonsignal/logging.c++',
  # redirect_service
  'neonsignal/redirect_service.c++',
  'neonsignal/redirect_service/close_connection_.c++',
  'neonsignal/redirect_service/handle_accept_.c++',
  'neonsignal/redirect_service/handle_io_.c++',
  'neonsignal/redirect_service/process_buffer_.c++',
  'neonsignal/redirect_service/register_connection_.c++',
  'neonsignal/redirect_service/serve_acme_challenge_.c++',
  'neonsignal/redirect_service/send_redirect_.c++',
  'neonsignal/redirect_service/setup_listener_.c++',
  'neonsignal/redirect_service/start.c++',
  'neonsignal/redirect_service/stop.c++',
  'redirect_main.c++',
]

executable('neonsignal',
  srcs,
  include_directories : neonsignal_inc,
  dependencies : [openssl_dep, nghttp2_dep, mdbx_dep],
  install : true
)

executable('neonsignal_redirect',
  redirect_srcs,
  include_directories : neonsignal_inc,
  install : true
)
