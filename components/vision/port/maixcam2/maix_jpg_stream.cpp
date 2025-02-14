/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2024.5.17: Add framework, create this file.
 */

#include "maix_jpg_stream.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <linux/socket.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>

#define BOUNDARY "frame"

static const char *default_index_str =
"<html>\n"
"<body>\n"
"<h1>JPG Stream</h1>\n"
"<img src='/stream'>\n"
"</body>\n"
"</html>";

typedef struct {
	int socket;
	int thread;
	int idx;
	uint8_t is_inited;
	uint8_t is_running;
	uint8_t try_update_buffer;
	uint8_t try_exit;
	pthread_mutex_t lock;
} client_info_t;

typedef struct {
	void *p;
	size_t size;
	uint8_t valid;
} buffer_info_t;

typedef struct {
	int socket_fd;
	struct sockaddr_in address;
	int addrlen;

	pthread_mutex_t lock;
	int thread;
	uint8_t thread_is_started;
	uint8_t try_exit_thread;

	int client_cnt;
	int client_max;
	client_info_t *client;

	buffer_info_t buffer[2];
	int new_buffer_idx;

	char *index_str;
} priv_t;

priv_t priv;


static char *get_ip_str(char *hostname) {
    struct addrinfo hints, *res, *p;
    int status;
    char *ipstr = (char *)malloc(INET6_ADDRSTRLEN);
	if (!ipstr) return NULL;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return NULL;
    }

    for (p = res; p != NULL; p = p->ai_next) {
        void *addr;
        if (p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
        }
        inet_ntop(p->ai_family, addr, ipstr, INET6_ADDRSTRLEN);
    }

    freeaddrinfo(res);

	return ipstr;
}

static int socket_is_connected(int sockfd) {
    char buffer;
    int result = recv(sockfd, &buffer, 1, MSG_PEEK | MSG_DONTWAIT);
    if (result == 0) {
        return 0;
    } else if (result < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 1;
        } else {
            return 0;
        }
    }
    return 1;
}

static size_t socket_write(int socket, void *data, size_t size)
{
	if (!socket_is_connected(socket)) {
		return -1;
	}
	return write(socket, data, size);
}

static size_t socket_read(int socket, void *data, size_t size)
{
	return read(socket, data, size);
}

void send_response(int client_socket, char *header, char *body) {
    socket_write(client_socket, header, strlen(header));
    socket_write(client_socket, body, strlen(body));
}

static int find_unused_idx(client_info_t *client, int client_max)
{
	for (int i = 0; i < client_max; i ++) {
		client_info_t *c = (client_info_t *)&client[i];
		if (!c->is_inited) {
			return i;
		}
	}
	return -1;
}

static int notify_client_update_buffer()
{
	for (int i = 0; i < priv.client_max; i ++) {
		client_info_t *c = (client_info_t *)&priv.client[i];
		if (c->is_inited) {
			pthread_mutex_lock(&c->lock);
			c->try_update_buffer = 1;
			pthread_mutex_unlock(&c->lock);
		}
	}

	return 0;
}

static int get_global_buffer_not_safe(void **buffer, size_t *size)
{
	buffer_info_t *info = (buffer_info_t *)&priv.buffer[priv.new_buffer_idx];
	if (info->valid) {
		if (!info->p) return -1;

		if (buffer) *buffer = info->p;
		if (size) *size = info->size;
		return 0;
	}

	return -1;
}

static void on_stream(client_info_t *client) {
	int client_socket = client->socket;
    char header[1024];
    sprintf(header, "HTTP/1.1 200 OK\r\n");
    sprintf(header + strlen(header), "Content-Type: multipart/x-mixed-replace; boundary=%s\r\n", BOUNDARY);
    sprintf(header + strlen(header), "\r\n");
    socket_write(client_socket, header, strlen(header));

	int update = 0;
    while (1) {
		pthread_mutex_lock(&client->lock);
		if (client->try_exit || !socket_is_connected(client->socket)) {
			pthread_mutex_unlock(&client->lock);
			break;
		}

		if (client->try_update_buffer) {
			update = 1;
			client->try_update_buffer = 0;
		}
		pthread_mutex_unlock(&client->lock);

		size_t image_size;
		char *image_data;
		if (update) {
			update = 0;

			pthread_mutex_lock(&priv.lock);
			void *buffer;
			size_t buffer_size;
			if (0 != get_global_buffer_not_safe(&buffer, &buffer_size)) {
				printf("get global buffer failed!\r\n");
				usleep(100*1000);
				continue;
			}

			image_size = buffer_size;
			image_data = (char *)malloc(image_size);
			if (!image_data) {
				printf("malloc failed!\r\n");
				usleep(100*1000);
				continue;
			}
			memcpy(image_data, buffer, image_size);
			pthread_mutex_unlock(&priv.lock);

			sprintf(header, "--%s\r\n", BOUNDARY);
			sprintf(header + strlen(header), "Content-Type: image/jpeg\r\n");
			sprintf(header + strlen(header), "Content-Length: %ld\r\n", image_size);
			sprintf(header + strlen(header), "\r\n");
			socket_write(client_socket, header, strlen(header));
			socket_write(client_socket, image_data, image_size);
			socket_write(client_socket, (char *)"\r\n", 2);
			free(image_data);
		} else {
			usleep(10 * 1000);
		}
    }
}

int http_jpeg_server_create(char *host, int port, int client_num) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
		close(server_fd);
        return -1;
    }

    address.sin_family = AF_INET;
	if (host == 0 || strlen(host) == 0) {
		address.sin_addr.s_addr = INADDR_ANY;
	} else {
		char *ip = (char *)get_ip_str(host);
		if (!ip) {
			printf("can not parse ip:%s\r\n", host);
			close(server_fd);
			return -1;
		}
		address.sin_addr.s_addr = inet_addr(ip);
		free(ip);
	}
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
		close(server_fd);
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
		close(server_fd);
        return -1;
    }

	if (0 != pthread_mutex_init(&priv.lock, NULL)) {
		printf("create lock failed!\r\n");
		close(server_fd);
		return -1;
	}

	priv.socket_fd = server_fd;
	memcpy(&priv.address, &address, sizeof(struct sockaddr_in));
	priv.addrlen = addrlen;
	priv.client_max = client_num;
	priv.client = (client_info_t *)malloc(priv.client_max * sizeof(client_info_t));
	if (priv.client == NULL) {
		printf("create client info failed!\r\n");
		return -1;
	}
	memset(priv.client, 0, priv.client_max * sizeof(client_info_t));
	memset(priv.buffer, 0, sizeof(priv.buffer));
	priv.client_cnt = 0;
	priv.index_str = (char *)default_index_str;
	return 0;
}

static void *client_thread_handle(void *param)
{
	int res;
	pthread_mutex_lock(&priv.lock);
	client_info_t *client = (client_info_t *)param;
	pthread_mutex_t *lock = &client->lock;
	pthread_mutex_unlock(&priv.lock);

	pthread_mutex_lock(lock);
	int client_socket = client->socket;
	client->is_running = 1;
	pthread_mutex_unlock(lock);

	fd_set readfds;
	while (1) {
		pthread_mutex_lock(lock);
		if (client->try_exit || !socket_is_connected(client->socket)) {
			pthread_mutex_unlock(lock);
			break;
		}
		pthread_mutex_unlock(lock);

		FD_ZERO(&readfds);
		FD_SET(client_socket, &readfds);

        char buffer[1024] = {0};

		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(client_socket, &fds);

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100 * 1000;
		int ret = select(client_socket + 1, &fds, NULL, NULL, &tv);
		if (ret == -1) {
			printf("select error! ret:%d\r\n", ret);
			continue;
		} else if (ret == 0) {
			continue;		// timeout
		} else {
			res = socket_read(client_socket, buffer, 1024);
			if (res < 0) {
				printf("socket_read failed! res: %d\r\n", res);
				break;
			}
		}

        if (strstr(buffer, "GET /stream") != NULL) {
			on_stream(client);
        } else {
            char *response = priv.index_str;
			socket_write(client_socket, (char *)"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n", strlen("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"));
    		socket_write(client_socket, response, strlen(response));
        }
	}

	pthread_mutex_lock(&priv.lock);
	if (client->is_inited) {
		close(client->socket);
		client->socket = -1;
		client->thread = -1;		// will exit
		client->is_running = 0;
		client->is_inited = 0;
		priv.client_cnt --;
		pthread_mutex_destroy(&client->lock);
	}
	pthread_mutex_unlock(&priv.lock);
	pthread_exit(NULL);
}

static void *thread_handle(void *param)
{
	int res;
	priv_t *priv = (priv_t *)param;
	int server_fd;

	pthread_mutex_lock(&priv->lock);
	server_fd = priv->socket_fd;
	pthread_mutex_unlock(&priv->lock);
	while (1) {
		pthread_mutex_lock(&priv->lock);
		if (priv->try_exit_thread) {
			pthread_mutex_unlock(&priv->lock);
			break;
		}
		pthread_mutex_unlock(&priv->lock);

		int client_socket;
        if ((client_socket = accept(server_fd, (struct sockaddr *)&priv->address, (socklen_t*)&priv->addrlen)) < 0) {
            perror("accept");
			sleep(1);
			continue;
        }

		pthread_mutex_lock(&priv->lock);
		int idx = find_unused_idx(priv->client, priv->client_max);
		if (idx < 0) {
			printf("can not create more client! curr:%d max:%d\r\n", priv->client_cnt, priv->client_max);
			continue;;
		}

		client_info_t *client = (client_info_t *)&priv->client[idx];
		client->socket = client_socket;
		client->try_exit = 0;
		client->idx = idx;
		if (0 != pthread_mutex_init(&client->lock, NULL)) {
			printf("create client lock failed!\r\n");
			continue;
		}

		if (0 != (res = pthread_create((pthread_t *)&client->thread, NULL, client_thread_handle, client))) {
			fprintf(stderr, "create client thread error:%s\n", strerror(res));
			pthread_mutex_destroy(&client->lock);
			continue;
		}

		// if (0 != (res = pthread_detach(&client->thread))) {
		// 	fprintf(stderr, "client thread detach error:%s\n", strerror(res));
		// 	pthread_mutex_destroy(&client->lock);
		// 	continue;
		// }
		client->is_inited = 1;
		priv->client_cnt ++;
		pthread_mutex_unlock(&priv->lock);
	}

	return NULL;
}

static int http_jpeg_server_set_index_str(char *index)
{
	if (priv.index_str != default_index_str) {
		if (priv.index_str) {
			free(priv.index_str);
			priv.index_str = NULL;
		}
	}

	priv.index_str = (char *)malloc(strlen(index) + 1);
	if (!priv.index_str) {
		printf("malloc failed!\r\n");
		return -1;
	}
	strcpy(priv.index_str, index);
	return 0;
}

static int http_jpeg_server_start() {
	pthread_t thread;

	pthread_mutex_lock(&priv.lock);
	if (priv.thread_is_started) {
		return 0;
	}

	priv.try_exit_thread = 0;
	if (0 != pthread_create(&thread, NULL, thread_handle, &priv)) {
		printf("create thread failed!\r\n");
		return -1;
	}

	priv.thread = thread;
	priv.thread_is_started = 1;
	pthread_mutex_unlock(&priv.lock);

	return 0;
}

static int http_jpeg_server_stop() {
	pthread_mutex_lock(&priv.lock);
	if (!priv.thread_is_started) {
		return 0;
	}
	priv.try_exit_thread = 1;
	pthread_mutex_unlock(&priv.lock);

	for (int i = 0; i < priv.client_max; i ++) {
		client_info_t *c = (client_info_t *)&priv.client[i];
		if (c->is_inited) {
			pthread_mutex_lock(&c->lock);
			c->try_exit = 1;
			pthread_mutex_unlock(&c->lock);
		}
	}

	// pthread_join(priv.thread, NULL);
	priv.thread_is_started = 0;
	return 0;
}


static int http_jpeg_server_destory() {
	http_jpeg_server_stop();

	if (priv.client) {
		free(priv.client);
		priv.client = NULL;
	}

	if (priv.socket_fd > 0) {
		close(priv.socket_fd);
		priv.socket_fd = -1;
	}

	pthread_mutex_destroy(&priv.lock);

	return 0;
}

static int http_jpeg_server_send(void *data, size_t size)
{
	pthread_mutex_lock(&priv.lock);
	int next_buffer_idx = !priv.new_buffer_idx;
	if (priv.buffer[next_buffer_idx].p) {
		free(priv.buffer[next_buffer_idx].p);
		priv.buffer[next_buffer_idx].p = NULL;
	}
	priv.buffer[next_buffer_idx].p = malloc(size);
	if (priv.buffer[next_buffer_idx].p == NULL) {
		printf("create new buffer failed!\r\n");
		return -1;
	}
	memcpy(priv.buffer[next_buffer_idx].p, data, size);
	priv.buffer[next_buffer_idx].size = size;
	priv.buffer[next_buffer_idx].valid = 1;
	priv.new_buffer_idx = next_buffer_idx;
	pthread_mutex_unlock(&priv.lock);

	notify_client_update_buffer();

	return 0;
}

namespace maix::http
{
        JpegStreamer::JpegStreamer(std::string host, int port, int client_number) {
			int res = 0;
			if (host.size() == 0) {
				host = "0.0.0.0";
			}

			if (0 != (res = http_jpeg_server_create((char *)host.c_str(), port, client_number))) {
				err::check_raise(err::ERR_RUNTIME, "http_jpeg_server_create failed!");
			}

			_host = host;
			_port = port;
		}

        JpegStreamer::~JpegStreamer() {
			int res = 0;
			if (0 != (res = http_jpeg_server_destory())) {
				err::check_raise(err::ERR_RUNTIME, "http_jpeg_server_start failed!");
			}
		}

        err::Err JpegStreamer::start() {
			int res = 0;
			if (0 != (res = http_jpeg_server_start())) {
				log::error("http_jpeg_server_start failed! res:%d\r\n",  res);
				return err::ERR_RUNTIME;
			}

			return err::ERR_NONE;
		}

        err::Err JpegStreamer::stop() {
			int res = 0;
			if (0 != (res = http_jpeg_server_stop())) {
				log::error("http_jpeg_server_stop failed! res:%d\r\n",  res);
				return err::ERR_RUNTIME;
			}

			return err::ERR_NONE;
		}

        err::Err JpegStreamer::write(image::Image *img) {
			int res = 0;
			image::Image *jpg = NULL;

			if (img->format() != image::Format::FMT_JPEG) {
				jpg = img->to_jpeg();
				if (jpg == NULL) {
					log::error("invert to jpeg failed!\r\n");
					return err::ERR_RUNTIME;
				}
			} else {
				jpg = img;
			}

			if (0 != (res = http_jpeg_server_send(jpg->data(), jpg->data_size()))) {
				log::error("http_jpeg_server_send failed! res:%d\r\n",  res);
				return err::ERR_RUNTIME;
			}

			if (img->format() != image::Format::FMT_JPEG) {
				delete jpg;
			}
			return err::ERR_NONE;
		}

        err::Err JpegStreamer::set_html(std::string data) {
			int res = 0;
			if (data.size() == 0) {
				log::error("html code is none!\r\n");
				return err::ERR_RUNTIME;
			}

			if (0 != (res = http_jpeg_server_set_index_str(&data[0]))) {
				log::error("http_jpeg_server_set_index_str failed! res:%d\r\n",  res);
				return err::ERR_RUNTIME;
			}

			return err::ERR_NONE;
		}
}