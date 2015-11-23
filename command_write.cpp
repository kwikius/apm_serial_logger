

bool command_append()
{
   return write_text_file(get_cmd_arg(1),O_WRITE | O_APPEND );
}

bool command_write()
{
   return write_text_file(get_cmd_arg(1),O_WRITE | O_TRUNC );
}

bool command_new()
{
   return write_text_file(get_cmd_arg(1),O_CREAT | O_EXCL | O_WRITE);
}

