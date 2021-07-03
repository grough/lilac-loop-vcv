# Looper Two Module Development

To enable the Looper Two module for development, add this object to the `modules` array in `plugin.json`:

```js
{
  "slug": "LooperTwo",
  "name": "Looper Two"
}
```

Build and run the test program:

```sh
# Build the test program
$ g++ -std=c++11 -Isrc/ test/test.cpp -o test/test.out

# Run the test program
$ ./test/test.out
```
