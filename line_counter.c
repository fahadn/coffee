int line_count_of(command_stream_t s, int index)
{
  if (index >= s->size)
    return 0;
  int line_count = 0;
  int i = 0;
  for (i = 0; i < s->size; i++)
  {
    if (strcmp(s->command_list[i], "\n"))
    {
      line_count++;
    }
  }
  return line_count;
}
