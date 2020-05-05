size_t growth_left = 0;
size_t capacity = vars.capacity();
size_t size_ = vars.size();
if (capacity == 7)
{
  // x-x/8 does not work when x==7.
  growth_left = 6 - size_;
}
else
{
  growth_left = (capacity - capacity / 8) - size_;
}
//phmap::container_internal::hashtable_debug_internal::HashtableDebugAccess::GetNumProbes;
//deb(vars.GetNumProbes);
deb(vars.capacity());
deb(vars.size());
deb(growth_left);
deb(vars.bucket_count());
deb(vars.load_factor());
std::cout << std::endl;
//if (size > (8 * capacity)/10) {
//  vars.resize(4 * capacity);
//}

//printf("size = %d\n", size);
//deb(var->get_read_id());
//deb(var->get_write_id());
//printf("sizeof(VectorClock) = %d\n", sizeof(VectorClock<>));
