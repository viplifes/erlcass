#include "data_conversion.h"
#include "erlcass.h"
#include "nif_utils.h"
#include "uuid_serialization.h"

#include <string.h>

ERL_NIF_TERM string_to_erlang_term(ErlNifEnv* env, const CassValue* value);
ERL_NIF_TERM blob_to_erlang_term(ErlNifEnv* env, const CassValue* value);
ERL_NIF_TERM uuid_to_erlang_term(ErlNifEnv* env, const CassValue* value);
ERL_NIF_TERM inet_to_erlang_term(ErlNifEnv* env, const CassValue* value);
ERL_NIF_TERM decimal_to_erlang_term(ErlNifEnv* env, const CassValue* value);
ERL_NIF_TERM date_to_erlang_term(ErlNifEnv* env, const CassValue* value);
ERL_NIF_TERM int64_to_erlang_term(ErlNifEnv* env, const CassValue* value);
ERL_NIF_TERM bool_to_erlang_term(const CassValue* value);
ERL_NIF_TERM float_to_erlang_term(ErlNifEnv* env, const CassValue* value);
ERL_NIF_TERM double_to_erlang_term(ErlNifEnv* env, const CassValue* value);
ERL_NIF_TERM tiny_int_to_erlang_term(ErlNifEnv* env, const CassValue* value);
ERL_NIF_TERM small_int_to_erlang_term(ErlNifEnv* env, const CassValue* value);
ERL_NIF_TERM int_to_erlang_term(ErlNifEnv* env, const CassValue* value);
ERL_NIF_TERM collection_to_erlang_term(ErlNifEnv* env, const CassValue* value);
ERL_NIF_TERM tuple_to_erlang_term(ErlNifEnv* env, const CassValue* value);
ERL_NIF_TERM udt_to_erlang_term(ErlNifEnv* env, const CassValue* value);

ERL_NIF_TERM cass_value_to_nif_term(ErlNifEnv* env, const CassValue* value)
{
    if(cass_value_is_null(value))
        return ATOMS.atomNull;

    switch (cass_value_type(value))
    {
        case CASS_VALUE_TYPE_ASCII:
        case CASS_VALUE_TYPE_TEXT:
        case CASS_VALUE_TYPE_VARCHAR:
            return string_to_erlang_term(env, value);

        case CASS_VALUE_TYPE_TIME:
        case CASS_VALUE_TYPE_TIMESTAMP:
        case CASS_VALUE_TYPE_COUNTER:
        case CASS_VALUE_TYPE_BIGINT:
            return int64_to_erlang_term(env, value);

        case CASS_VALUE_TYPE_BOOLEAN:
            return bool_to_erlang_term(value);

        case CASS_VALUE_TYPE_FLOAT:
            return float_to_erlang_term(env, value);

        case CASS_VALUE_TYPE_DOUBLE:
            return double_to_erlang_term(env, value);

        case CASS_VALUE_TYPE_TINY_INT:
            return tiny_int_to_erlang_term(env, value);

        case CASS_VALUE_TYPE_SMALL_INT:
            return small_int_to_erlang_term(env, value);

        case CASS_VALUE_TYPE_INT:
            return int_to_erlang_term(env, value);

        case CASS_VALUE_TYPE_DATE:
            return date_to_erlang_term(env, value);

        case CASS_VALUE_TYPE_VARINT:
        case CASS_VALUE_TYPE_BLOB:
            return blob_to_erlang_term(env, value);

        case CASS_VALUE_TYPE_TIMEUUID:
        case CASS_VALUE_TYPE_UUID:
            return uuid_to_erlang_term(env, value);

        case CASS_VALUE_TYPE_INET:
            return inet_to_erlang_term(env, value);

        case CASS_VALUE_TYPE_DECIMAL:
            return decimal_to_erlang_term(env, value);

        case CASS_VALUE_TYPE_SET:
        case CASS_VALUE_TYPE_LIST:
        case CASS_VALUE_TYPE_MAP:
            return collection_to_erlang_term(env, value);

        case CASS_VALUE_TYPE_TUPLE:
            return tuple_to_erlang_term(env, value);

        case CASS_VALUE_TYPE_UDT:
            return udt_to_erlang_term(env, value);

        default:
            //unsuported types and null values
            return ATOMS.atomNull;
    }
}

ERL_NIF_TERM string_to_erlang_term(ErlNifEnv* env, const CassValue* value)
{
    const char* buff_ptr;
    size_t buff_size;
    cass_value_get_string(value, &buff_ptr, &buff_size);
    return make_binary(env, buff_ptr, buff_size);
}

ERL_NIF_TERM blob_to_erlang_term(ErlNifEnv* env, const CassValue* value)
{
    const cass_byte_t * buffer = NULL;
    size_t buffer_size;
    cass_value_get_bytes(value, &buffer, &buffer_size);
    return make_binary(env, (const char*)buffer, buffer_size);
}

ERL_NIF_TERM uuid_to_erlang_term(ErlNifEnv* env, const CassValue* value)
{
    CassUuid uuid;
    cass_value_get_uuid(value, &uuid);

    char uuid_str[CASS_UUID_STRING_LENGTH];
    erlcass::cass_uuid_string(uuid, uuid_str);

    return make_binary(env, uuid_str, CASS_UUID_STRING_LENGTH - 1);
}

ERL_NIF_TERM inet_to_erlang_term(ErlNifEnv* env, const CassValue* value)
{
    CassInet inet;
    cass_value_get_inet(value, &inet);

    char inet_str[CASS_INET_STRING_LENGTH];
    cass_inet_string(inet, inet_str);

    return make_binary(env, inet_str, strlen(inet_str));
}

ERL_NIF_TERM decimal_to_erlang_term(ErlNifEnv* env, const CassValue* value)
{
    const cass_byte_t * buffer = NULL;
    size_t buffer_size;
    cass_int32_t scale;
    cass_value_get_decimal(value, &buffer, &buffer_size, &scale);

    return enif_make_tuple2(env, make_binary(env, reinterpret_cast<const char*>(buffer), buffer_size), enif_make_int(env, scale));
}

ERL_NIF_TERM int64_to_erlang_term(ErlNifEnv* env, const CassValue* value)
{
    cass_int64_t value_long;
    cass_value_get_int64(value, &value_long);
    return enif_make_int64(env, value_long);
}

ERL_NIF_TERM bool_to_erlang_term(const CassValue* value)
{
    cass_bool_t bool_val;
    cass_value_get_bool(value, &bool_val);
    return bool_val ? ATOMS.atomTrue : ATOMS.atomFalse;
}

ERL_NIF_TERM float_to_erlang_term(ErlNifEnv* env, const CassValue* value)
{
    cass_float_t value_float;
    cass_value_get_float(value, &value_float);
    return enif_make_double(env, value_float);
}

ERL_NIF_TERM double_to_erlang_term(ErlNifEnv* env, const CassValue* value)
{
    cass_double_t value_double;
    cass_value_get_double(value, &value_double);
    return enif_make_double(env, value_double);
}

ERL_NIF_TERM date_to_erlang_term(ErlNifEnv* env, const CassValue* value)
{
    cass_uint32_t value_uint;
    cass_value_get_uint32(value, &value_uint);
    return enif_make_uint(env, value_uint);
}

ERL_NIF_TERM int_to_erlang_term(ErlNifEnv* env, const CassValue* value)
{
    cass_int32_t value_int;
    cass_value_get_int32(value, &value_int);
    return enif_make_int(env, value_int);
}

ERL_NIF_TERM tiny_int_to_erlang_term(ErlNifEnv* env, const CassValue* value)
{
    cass_int8_t value_int;
    cass_value_get_int8(value, &value_int);
    return enif_make_int(env, value_int);
}

ERL_NIF_TERM small_int_to_erlang_term(ErlNifEnv* env, const CassValue* value)
{
    cass_int16_t value_int;
    cass_value_get_int16(value, &value_int);
    return enif_make_int(env, value_int);
}

ERL_NIF_TERM collection_to_erlang_term(ErlNifEnv* env, const CassValue* value)
{
    size_t itemsCount = cass_value_item_count(value);

    if(itemsCount == 0)
        return enif_make_list(env, 0);

    ERL_NIF_TERM itemsList[itemsCount];
    size_t rowIndex = 0;

    if(cass_value_type(value) == CASS_VALUE_TYPE_MAP)
    {
        CassIterator* iterator = cass_iterator_from_map(value);

        while (cass_iterator_next(iterator))
        {
            const CassValue* c_key = cass_iterator_get_map_key(iterator);
            const CassValue* c_value = cass_iterator_get_map_value(iterator);
            itemsList[rowIndex++] = enif_make_tuple2(env, cass_value_to_nif_term(env, c_key), cass_value_to_nif_term(env, c_value));
        }

        cass_iterator_free(iterator);
    }
    else
    {
        CassIterator* iterator = cass_iterator_from_collection(value);

        while (cass_iterator_next(iterator))
            itemsList[rowIndex++] = cass_value_to_nif_term(env, cass_iterator_get_value(iterator));

        cass_iterator_free(iterator);
    }

    return enif_make_list_from_array(env, itemsList, static_cast<unsigned>(itemsCount));
}

ERL_NIF_TERM tuple_to_erlang_term(ErlNifEnv* env, const CassValue* value)
{
    size_t itemsCount = cass_value_item_count(value);

    if(itemsCount == 0)
        return enif_make_tuple(env, 0);

    ERL_NIF_TERM itemsList[itemsCount];
    size_t rowIndex = 0;

    CassIterator* iterator = cass_iterator_from_tuple(value);

    while (cass_iterator_next(iterator))
        itemsList[rowIndex++] = cass_value_to_nif_term(env, cass_iterator_get_value(iterator));

    cass_iterator_free(iterator);

    return enif_make_tuple_from_array(env, itemsList, static_cast<unsigned>(itemsCount));
}

ERL_NIF_TERM udt_to_erlang_term(ErlNifEnv* env, const CassValue* value)
{
    size_t items_count = cass_value_item_count(value);

    if(items_count == 0)
        return ATOMS.atomNull;

    ERL_NIF_TERM items_list[items_count];
    size_t rowIndex = 0;

    CassIterator* iterator = cass_iterator_fields_from_user_type(value);

    while (cass_iterator_next(iterator))
    {
        const char* field_name_ptr;
        size_t field_name_length;

        if(cass_iterator_get_user_type_field_name(iterator, &field_name_ptr, &field_name_length) != CASS_OK)
            return ATOMS.atomNull;

        ERL_NIF_TERM field_value = cass_value_to_nif_term(env, cass_iterator_get_user_type_field_value(iterator));
        items_list[rowIndex++] = enif_make_tuple2(env, make_binary(env, field_name_ptr, field_name_length), field_value);
    }

    cass_iterator_free(iterator);

    return enif_make_tuple2(env, ATOMS.atomDataTypeUdt, enif_make_list_from_array(env, items_list, items_count));
}

//convert CassResult into erlang term

ERL_NIF_TERM cass_result_to_erlang_term(ErlNifEnv* env, const CassResult* result)
{
    size_t rowsCount = cass_result_row_count(result);

    if(rowsCount == 0)
        return enif_make_list(env, 0);

    size_t columnsCount = cass_result_column_count(result);

    ERL_NIF_TERM nifArrayColumns[columnsCount];
    ERL_NIF_TERM nifArrayRows[rowsCount];

    CassIterator* iterator = cass_iterator_from_result(result);

    size_t rowIndex = 0;

    while (cass_iterator_next(iterator))
    {
        const CassRow* row = cass_iterator_get_row(iterator);

        for(size_t i = 0; i < columnsCount; i++)
            nifArrayColumns[i] = cass_value_to_nif_term(env, cass_row_get_column(row, i));

        nifArrayRows[rowIndex++] = enif_make_tuple_from_array(env, nifArrayColumns, static_cast<unsigned>(columnsCount));
    }

    cass_iterator_free(iterator);

    return enif_make_list_from_array(env, nifArrayRows, static_cast<unsigned>(rowsCount));
}
