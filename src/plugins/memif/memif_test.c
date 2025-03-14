/*
 * memif VAT support
 *
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <inttypes.h>

#include <vat/vat.h>
#include <vlibapi/api.h>
#include <vlibmemory/api.h>

#include <vppinfra/error.h>
#include <vnet/ip/ip.h>
#include <memif/memif.h>
#include <memif/private.h>

#define __plugin_msg_base memif_test_main.msg_id_base
#include <vlibapi/vat_helper_macros.h>

/* declare message IDs */
#include <vnet/format_fns.h>
#include <memif/memif.api_enum.h>
#include <memif/memif.api_types.h>
#include <vlibmemory/vlib.api_types.h>

typedef struct
{
  /* API message ID base */
  u16 msg_id_base;
  u32 ping_id;
  vat_main_t *vat_main;
} memif_test_main_t;

memif_test_main_t memif_test_main;

static uword
unformat_memif_queues (unformat_input_t * input, va_list * args)
{
  u32 *rx_queues = va_arg (*args, u32 *);
  u32 *tx_queues = va_arg (*args, u32 *);

  if (unformat (input, "rx-queues %u", rx_queues))
    ;
  if (unformat (input, "tx-queues %u", tx_queues))
    ;

  return 1;
}

/* memif_socket_filename_add_del API */
static int
api_memif_socket_filename_add_del (vat_main_t * vam)
{
  unformat_input_t *i = vam->input;
  vl_api_memif_socket_filename_add_del_t *mp;
  u8 is_add;
  u32 socket_id;
  u8 *socket_filename;
  int ret;

  is_add = 1;
  socket_id = ~0;
  socket_filename = 0;

  while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (i, "id %u", &socket_id))
	;
      else if (unformat (i, "filename %s", &socket_filename))
	;
      else if (unformat (i, "del"))
	is_add = 0;
      else if (unformat (i, "add"))
	is_add = 1;
      else
	{
	  vec_free (socket_filename);
	  clib_warning ("unknown input `%U'", format_unformat_error, i);
	  return -99;
	}
    }

  if (socket_id == 0 || socket_id == ~0)
    {
      vec_free (socket_filename);
      errmsg ("Invalid socket id");
      return -99;
    }

  if (is_add && (!socket_filename || *socket_filename == 0))
    {
      vec_free (socket_filename);
      errmsg ("Invalid socket filename");
      return -99;
    }

  M2 (MEMIF_SOCKET_FILENAME_ADD_DEL, mp, strlen ((char *) socket_filename));

  mp->is_add = is_add;
  mp->socket_id = htonl (socket_id);
  char *p = (char *) &mp->socket_filename;
  p += vl_api_vec_to_api_string (socket_filename, (vl_api_string_t *) p);

  vec_free (socket_filename);

  S (mp);
  W (ret);

  return ret;
}

/* memif_socket_filename_add_del API */
static int
api_memif_socket_filename_add_del_v2 (vat_main_t *vam)
{
  unformat_input_t *i = vam->input;
  vl_api_memif_socket_filename_add_del_v2_t *mp;
  u8 is_add;
  u32 socket_id;
  u8 *socket_filename;
  int ret;

  is_add = 1;
  socket_id = ~0;
  socket_filename = 0;

  while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (i, "id %u", &socket_id))
	;
      else if (unformat (i, "filename %s", &socket_filename))
	;
      else if (unformat (i, "del"))
	is_add = 0;
      else if (unformat (i, "add"))
	is_add = 1;
      else
	{
	  vec_free (socket_filename);
	  clib_warning ("unknown input `%U'", format_unformat_error, i);
	  return -99;
	}
    }

  if (socket_id == 0 || socket_id == ~0)
    {
      vec_free (socket_filename);
      errmsg ("Invalid socket id");
      return -99;
    }

  if (is_add && (!socket_filename || *socket_filename == 0))
    {
      vec_free (socket_filename);
      errmsg ("Invalid socket filename");
      return -99;
    }

  M2 (MEMIF_SOCKET_FILENAME_ADD_DEL_V2, mp, strlen ((char *) socket_filename));

  mp->is_add = is_add;
  mp->socket_id = htonl (socket_id);
  char *p = (char *) &mp->socket_filename;
  p += vl_api_vec_to_api_string (socket_filename, (vl_api_string_t *) p);

  vec_free (socket_filename);

  S (mp);
  W (ret);

  return ret;
}

/* memif socket-create reply handler */
static void
vl_api_memif_socket_filename_add_del_v2_reply_t_handler (
  vl_api_memif_socket_filename_add_del_v2_reply_t *mp)
{
  vat_main_t *vam = memif_test_main.vat_main;
  i32 retval = ntohl (mp->retval);

  if (retval == 0)
    {
      fformat (vam->ofp, "created memif socket with socket_id %d\n",
	       ntohl (mp->socket_id));
    }

  vam->retval = retval;
  vam->result_ready = 1;
}

/* memif_socket_filename_add_del reply handler */
#define VL_API_MEMIF_SOCKET_FILENAME_ADD_DEL_REPLY_T_HANDLER
static void vl_api_memif_socket_filename_add_del_reply_t_handler
  (vl_api_memif_socket_filename_add_del_reply_t * mp)
{
  vat_main_t *vam = memif_test_main.vat_main;
  i32 retval = ntohl (mp->retval);

  vam->retval = retval;
  vam->result_ready = 1;
  vam->regenerate_interface_table = 1;
}

/* memif-create API */
static int
api_memif_create (vat_main_t * vam)
{
  unformat_input_t *i = vam->input;
  vl_api_memif_create_t *mp;
  u32 id = 0;
  u32 socket_id = 0;
  u8 *secret = 0;
  u8 role = 1;
  u32 ring_size = 0;
  u32 buffer_size = 0;
  u8 hw_addr[6] = { 0 };
  u32 rx_queues = MEMIF_DEFAULT_RX_QUEUES;
  u32 tx_queues = MEMIF_DEFAULT_TX_QUEUES;
  int ret;
  u8 mode = MEMIF_INTERFACE_MODE_ETHERNET;

  while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (i, "id %u", &id))
	;
      else if (unformat (i, "socket-id %u", &socket_id))
	;
      else if (unformat (i, "secret %s", &secret))
	;
      else if (unformat (i, "ring_size %u", &ring_size))
	;
      else if (unformat (i, "buffer_size %u", &buffer_size))
	;
      else if (unformat (i, "master"))
	role = 0;
      else if (unformat (i, "slave %U",
			 unformat_memif_queues, &rx_queues, &tx_queues))
	role = 1;
      else if (unformat (i, "mode ip"))
	mode = MEMIF_INTERFACE_MODE_IP;
      else if (unformat (i, "hw_addr %U", unformat_ethernet_address, hw_addr))
	;
      else
	{
	  clib_warning ("unknown input '%U'", format_unformat_error, i);
	  return -99;
	}
    }

  if (socket_id == ~0)
    {
      errmsg ("invalid socket-id\n");
      return -99;
    }

  if (!is_pow2 (ring_size))
    {
      errmsg ("ring size must be power of 2\n");
      return -99;
    }

  if (rx_queues > 255 || rx_queues < 1)
    {
      errmsg ("rx queue must be between 1 - 255\n");
      return -99;
    }

  if (tx_queues > 255 || tx_queues < 1)
    {
      errmsg ("tx queue must be between 1 - 255\n");
      return -99;
    }

  M2 (MEMIF_CREATE, mp, strlen ((char *) secret));

  mp->mode = mode;
  mp->id = clib_host_to_net_u32 (id);
  mp->role = role;
  mp->ring_size = clib_host_to_net_u32 (ring_size);
  mp->buffer_size = clib_host_to_net_u16 (buffer_size & 0xffff);
  mp->socket_id = clib_host_to_net_u32 (socket_id);
  if (secret != 0)
    {
      char *p = (char *) &mp->secret;
      p += vl_api_vec_to_api_string (secret, (vl_api_string_t *) p);
      vec_free (secret);
    }
  memcpy (mp->hw_addr, hw_addr, 6);
  mp->rx_queues = rx_queues;
  mp->tx_queues = tx_queues;

  S (mp);
  W (ret);
  return ret;
}

/* memif-create reply handler */
static void vl_api_memif_create_reply_t_handler
  (vl_api_memif_create_reply_t * mp)
{
  vat_main_t *vam = memif_test_main.vat_main;
  i32 retval = ntohl (mp->retval);

  if (retval == 0)
    {
      fformat (vam->ofp, "created memif with sw_if_index %d\n",
	       ntohl (mp->sw_if_index));
    }

  vam->retval = retval;
  vam->result_ready = 1;
  vam->regenerate_interface_table = 1;
}

/* memif-delete API */
static int
api_memif_delete (vat_main_t * vam)
{
  unformat_input_t *i = vam->input;
  vl_api_memif_delete_t *mp;
  u32 sw_if_index = 0;
  u8 index_defined = 0;
  int ret;

  while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (i, "sw_if_index %u", &sw_if_index))
	index_defined = 1;
      else
	{
	  clib_warning ("unknown input '%U'", format_unformat_error, i);
	  return -99;
	}
    }

  if (!index_defined)
    {
      errmsg ("missing sw_if_index\n");
      return -99;
    }

  M (MEMIF_DELETE, mp);

  mp->sw_if_index = clib_host_to_net_u32 (sw_if_index);

  S (mp);
  W (ret);
  return ret;
}

/* memif-dump API */
static int
api_memif_dump (vat_main_t * vam)
{
  memif_test_main_t *mm = &memif_test_main;
  vl_api_memif_dump_t *mp;
  vl_api_control_ping_t *mp_ping;
  int ret;

  if (vam->json_output)
    {
      clib_warning ("JSON output not supported for memif_dump");
      return -99;
    }

  M (MEMIF_DUMP, mp);
  S (mp);

  /* Use a control ping for synchronization */
  if (!mm->ping_id)
    mm->ping_id = vl_msg_api_get_msg_index ((u8 *) (VL_API_CONTROL_PING_CRC));
  mp_ping = vl_msg_api_alloc_as_if_client (sizeof (*mp_ping));
  mp_ping->_vl_msg_id = htons (mm->ping_id);
  mp_ping->client_index = vam->my_client_index;

  fformat (vam->ofp, "Sending ping id=%d\n", mm->ping_id);

  vam->result_ready = 0;
  S (mp_ping);

  W (ret);
  return ret;
}

/* memif-details message handler */
static void
vl_api_memif_details_t_handler (vl_api_memif_details_t * mp)
{
  vat_main_t *vam = memif_test_main.vat_main;

  fformat (vam->ofp, "%s: sw_if_index %u mac %U\n"
	   "   id %u socket-id %u role %s\n"
	   "   ring_size %u buffer_size %u\n"
	   "   state %s link %s\n",
	   mp->if_name, ntohl (mp->sw_if_index), format_ethernet_address,
	   mp->hw_addr, clib_net_to_host_u32 (mp->id),
	   clib_net_to_host_u32 (mp->socket_id),
	   mp->role ? "slave" : "master",
	   ntohl (mp->ring_size), ntohs (mp->buffer_size),
	   (mp->flags & IF_STATUS_API_FLAG_ADMIN_UP) ? "up" : "down",
	   (mp->flags & IF_STATUS_API_FLAG_LINK_UP) ? "up" : "down");
}

/* memif_socket_filename_dump API */
static int
api_memif_socket_filename_dump (vat_main_t * vam)
{
  memif_test_main_t *mm = &memif_test_main;
  vl_api_memif_socket_filename_dump_t *mp;
  vl_api_control_ping_t *mp_ping;
  int ret;

  if (vam->json_output)
    {
      clib_warning
	("JSON output not supported for memif_socket_filename_dump");
      return -99;
    }

  M (MEMIF_SOCKET_FILENAME_DUMP, mp);
  S (mp);

  /* Use a control ping for synchronization */
  if (!mm->ping_id)
    mm->ping_id = vl_msg_api_get_msg_index ((u8 *) (VL_API_CONTROL_PING_CRC));
  mp_ping = vl_msg_api_alloc_as_if_client (sizeof (*mp_ping));
  mp_ping->_vl_msg_id = htons (mm->ping_id);
  mp_ping->client_index = vam->my_client_index;

  fformat (vam->ofp, "Sending ping id=%d\n", mm->ping_id);

  vam->result_ready = 0;
  S (mp_ping);

  W (ret);
  return ret;
}

/* memif_socket_format_details message handler */
static void vl_api_memif_socket_filename_details_t_handler
  (vl_api_memif_socket_filename_details_t * mp)
{
  vat_main_t *vam = memif_test_main.vat_main;

  fformat (vam->ofp,
	   "id %u : filename %s\n",
	   ntohl (mp->socket_id), mp->socket_filename);
}

#include <memif/memif.api_test.c>

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "gnu")
 * End:
 */
