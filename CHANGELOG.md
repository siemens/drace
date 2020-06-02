#### 1.7.1 (2020-05-08)

##### Chores

*  cleanup in improved filtering ([915c6c8b](https://github.com/siemens/drace/commit/915c6c8b96a06bf49f6014f259a202f7d423af4d))

##### New Features

*  filter races that happen in an excluded module ([65514318](https://github.com/siemens/drace/commit/6551431840932aa73741e9549a761f09f7a62d65))

##### Bug Fixes

*  avoid false positive in static initialization ([adf40eac](https://github.com/siemens/drace/commit/adf40eacb7a693cbb6db5f6339482fefdf9462de))
*  repair no-instrumentation support ([14c73fb8](https://github.com/siemens/drace/commit/14c73fb8688aa928b2a71680fa7562f221f5e4db))
*  correctly handle faults in application ([1114dce2](https://github.com/siemens/drace/commit/1114dce2150d71af30ff8348af9a2a15bec31873))
*  validate dr path against dr 8.x ([286850e5](https://github.com/siemens/drace/commit/286850e564e4f3841527513e44d671d96d2a0535))

##### Other Changes

*  check validity of tids in join event ([d4459faa](https://github.com/siemens/drace/commit/d4459faa683a022eb47e8c45e48384ce20f7ab48))
* cout ([3cdaa6b9](https://github.com/siemens/drace/commit/3cdaa6b906bf05891011ce0ada3c08f623e5d072))

##### Tests

*  add linux support for segfault test ([de57243f](https://github.com/siemens/drace/commit/de57243f260771b1b1491d11e3da12c61fcab59b))
*  vaildate shoppingrush example from howto guide ([1baf7305](https://github.com/siemens/drace/commit/1baf7305d6e796f9d4320cf41085f38c4baa9379))

#### 1.7.0 (2020-05-05)

##### Chores

*  reduce necessary permissions for SHM ([4606dcc7](https://github.com/siemens/drace/commit/4606dcc7ed9a5dd09b9202950a7782a39af45a43))
*  speedup ci by using pre-build images ([a44a6fa8](https://github.com/siemens/drace/commit/a44a6fa8b7a94f5b1afa84392c0ef0f583e76877))
*  remove is_standard_layout trait, as not working in all compilers ([674e73ac](https://github.com/siemens/drace/commit/674e73acd1586f7c9fab18ee7c25ad35c0e068c4))

##### Continuous Integration

*  use DynamoRio 8.0.x ([4d9b3e99](https://github.com/siemens/drace/commit/4d9b3e99c80081b77d489b844579a92793e6d839))
*  run package task in container ([b539244f](https://github.com/siemens/drace/commit/b539244f595cbe987eeed8abebb1382eacd2910c))
*  use variables to control dr version ([87d25dce](https://github.com/siemens/drace/commit/87d25dcee5a2519dedc9a1197dbaf26a98353323))

##### New Features

*  add printer detector to trace calls to the detector API ([85be7a50](https://github.com/siemens/drace/commit/85be7a50fb7d7d7fc36db1bfeda6061833c24b21))
*  add support to exclude addresses ([f19eac09](https://github.com/siemens/drace/commit/f19eac0948e6cd516d7da7ad82b98c37c5a77e49))
*  add addr-based race suppressions ([b09abe4f](https://github.com/siemens/drace/commit/b09abe4fe6a633679ab12536fb1d0d59f4b53ed2))

##### Bug Fixes

*  load tsan early as it uses TLS ([5b854111](https://github.com/siemens/drace/commit/5b854111d14fd21eca2e40303659ec5290b658d3))
*  use session-local events instead of global ones ([1c684295](https://github.com/siemens/drace/commit/1c6842953572f2e9e47ae3bad082b6a4f1b34cfe))
*  do not print after return in printer ([cbd9d53e](https://github.com/siemens/drace/commit/cbd9d53eb6b9d2f30107beb591eca16aefe71484))
*  thread-safety in address exclusion component ([7911c4fd](https://github.com/siemens/drace/commit/7911c4fd1f454d588d3cdd449d2eacd8e2d88df0))

##### Other Changes

*  build and publish doc in CI ([c4f11ac8](https://github.com/siemens/drace/commit/c4f11ac8372985ea1624408d1c775ae55a640959))
*  improve documentation regarding thread-safety guarantees ([1cc308a4](https://github.com/siemens/drace/commit/1cc308a4daefa36611ceba446d5ee4bceb817e90))

##### Performance Improvements

*  remove inheritance in spinlock ([ee67b9af](https://github.com/siemens/drace/commit/ee67b9af9958304a6ab7b48a197461cac4ad2e38))

##### Tests

*  make system tests portable ([c88499e3](https://github.com/siemens/drace/commit/c88499e353da28cc50e4d0fdb015be632c82e385))
*  streamlined ci ([a10e9157](https://github.com/siemens/drace/commit/a10e91579b8e3ed9f11e4ad49d9c8a69333c5d62))
*  disable tsan tests in CI ([dd372986](https://github.com/siemens/drace/commit/dd3729862759fe968c97fa69c7eba8ea04a3128a))
*  tweak to support unit tests in debug mode ([c8dacef7](https://github.com/siemens/drace/commit/c8dacef7d906372f8c30da9b2470780a82714cdf))

#### 1.6.1 (2020-03-26)

##### Chores

*  fix install location of drace config files ([ba1ab105](https://github.com/siemens/drace/commit/ba1ab1053a366684cce19e3b06fb25b3def05a93))

##### Refactors

*  minor changes to improve code readability ([a593bc82](https://github.com/siemens/drace/commit/a593bc82dba2ecd8049deb96944dfcd868afe967))
*  use wrapper around TLS ([d25e8ee0](https://github.com/siemens/drace/commit/d25e8ee0a2a4fdfb7ded3bf5605b01d08e7a65cf))
*  flatten ShadowThreadState ([e5864985](https://github.com/siemens/drace/commit/e586498502eb23091e7dc05c79d302c355112ec1))
*  cleanup global variables ([b9a6683a](https://github.com/siemens/drace/commit/b9a6683a92712de0d99fb0c01d7c42d249d83050))
*  carve out ShadowThreadState ([706c8d00](https://github.com/siemens/drace/commit/706c8d00e4f1ec5d15809a19071069cadf2de69c))
*  move memory limit specs to memory-tracker ([7c24e896](https://github.com/siemens/drace/commit/7c24e896b2d93753ae7f12c57f20ab74b36726bb))
*  carve out configuration modules ([16b23ab6](https://github.com/siemens/drace/commit/16b23ab6fd75d79cb714ef9d6dd9a8beed0e87cf))
*  move resp of tls to memory-tracker ([ec4bbd2d](https://github.com/siemens/drace/commit/ec4bbd2dfae986c2758af28e157110a3d81994fe))
*  further reduce globals ([8ec96366](https://github.com/siemens/drace/commit/8ec9636652fd66ac055f9e34094bb39b4c768c1c))
*  carve out global variables into dedicated components ([b0299798](https://github.com/siemens/drace/commit/b02997988be5aa582e96625eee33ab3783aba71c))
* **linux:**  add missing memory header ([5e91f341](https://github.com/siemens/drace/commit/5e91f341484aae6b8db00ef1760b9415dfd78eb7))

##### Code Style Changes

*  clang-format of every C or C++ source file (*.hpp, *.h, *.cpp) ([e1bbd482](https://github.com/siemens/drace/commit/e1bbd482130273c43f2839c066ecaab9e8cde388))

#### 1.6.0 (2020-02-05)

##### Chores

*  do not install googletest along with drace ([40ec63b7](https://github.com/siemens/drace/commit/40ec63b73ec74db48e0fa412cdd4ff9f80531d53))
*  unified cmake syntax to build tests ([61f2470d](https://github.com/siemens/drace/commit/61f2470d12e020e9616388548e27abfe6c76d6e6))

##### Continuous Integration

*  added linux built and test steps for x64 and i386 to pipeline ([f8824631](https://github.com/siemens/drace/commit/f8824631a3c32f8ce121ad0a798fed9f08b068c8))

##### Documentation Changes

*  finalized drace tutorial ([67d9be4f](https://github.com/siemens/drace/commit/67d9be4f3ffcdff92c068ca9713543b316a2f9f3))
*  cleanup readme and documentation ([0beafdd7](https://github.com/siemens/drace/commit/0beafdd72293433b8e7839607c9829babbb3ccd1))
*  first draft of a tutorial for drace ([acdd3ef6](https://github.com/siemens/drace/commit/acdd3ef602c3619c5838a3ac9b3f2e888a37c085))
*  make project reuse compliant ([87d7bb57](https://github.com/siemens/drace/commit/87d7bb57281788841514f4e558022373ea90adf2))

##### New Features

*  in race-report, put symbol name into dedicated entry ([65eefb00](https://github.com/siemens/drace/commit/65eefb00a41b8371668b6bf863b6ea3e271a5b9b))
*  added linux mutex names to drace, start tackling [#47](https://github.com/siemens/drace/pull/47) ([e8fbc7a2](https://github.com/siemens/drace/commit/e8fbc7a2f1ab3085cc504e67b8903ea3b518601b))
*  added linux mutex names ([d1a1e9c8](https://github.com/siemens/drace/commit/d1a1e9c816d0985da9d43b4b3a13f8673e51176c))
*  added support for valgrind/helgrind reports [#48](https://github.com/siemens/drace/pull/48) ([78b5c701](https://github.com/siemens/drace/commit/78b5c70127b190c827d241471d8858ee68c9690c))
*  added wild-card matching for module names [#46](https://github.com/siemens/drace/pull/46) ([aa937846](https://github.com/siemens/drace/commit/aa9378465bc2c3094a2c87f79e83433bfe83b3b0))
*  added support for valgrind/helgrind reports [#48](https://github.com/siemens/drace/pull/48) ([2eebd17c](https://github.com/siemens/drace/commit/2eebd17cfc181c1032729fc0581717d8d649b395))
*  adjusted error messages for report converter, tackles [#41](https://github.com/siemens/drace/pull/41) ([0b6ffa5e](https://github.com/siemens/drace/commit/0b6ffa5ed32af3466754b7bb63eb3dec20654c4d))
*  initial (experimental) support for drace on x64 linux ([2a1d3317](https://github.com/siemens/drace/commit/2a1d331781a7cbcd9199bd2741053c5b4fa5ba62))

##### Bug Fixes

*  use dedicated field for line number in MSR ([723b4fe5](https://github.com/siemens/drace/commit/723b4fe53859c31b5bb3efcb0b7b10965ca0727e))
*  properly terminate symbol c strings ([e5bb1ded](https://github.com/siemens/drace/commit/e5bb1ded9a64171d01e6c2832970379a019b7d75))
*  resolved ccp-check issue of shoppingrush ([80d0502f](https://github.com/siemens/drace/commit/80d0502ff454c5d8d2e72f21599b8e458cb54322))
*  manually link libatomic when using clang++ ([824163d3](https://github.com/siemens/drace/commit/824163d367b803f5c2d2b4d9b520d79bc4aa091f))
*  use getter to work around odr issues on constexpr ([47ac1492](https://github.com/siemens/drace/commit/47ac14927edccff9b31e74b627e9e6141171abc8))
*  upper memory space limit on windows ([ba734540](https://github.com/siemens/drace/commit/ba734540946219dbfeed98c3cd24bc9e6d8d1552))
*  upper memory space limit on windows ([cd3a540f](https://github.com/siemens/drace/commit/cd3a540f2ebce0e7831fc31961c38463f879618e))
*  solve multiple crashes in drace on linux ([2bbf8483](https://github.com/siemens/drace/commit/2bbf848389a0945e1f7609addb57657001d86c15))
*  carved out many linux-only bugs ([f1cbeab0](https://github.com/siemens/drace/commit/f1cbeab098952d345eed73dfb69b30ab476de5df))
*  removed template character (backtick) from created report to support IE ([2ac0b5f5](https://github.com/siemens/drace/commit/2ac0b5f57e5bce2fb1ab109dbe3e9332006612ad))

##### Other Changes

*  use embedded properties ([6957656a](https://github.com/siemens/drace/commit/6957656a0de27b3cc6b89dab915aad7f1649f625))
*  extended HowTo, added pictures, added example app ([260c4719](https://github.com/siemens/drace/commit/260c47196d4c1c0d4d277dc5c51f9254b4141f0a))
*  update on howto ([729d4983](https://github.com/siemens/drace/commit/729d4983856732776ed74a37b41d27598ab7aef4))
* multicore/drace into feature/dracelinux ([7303d9c0](https://github.com/siemens/drace/commit/7303d9c0c17c3a1e9692648ad2ecf75114a2afd8))
* multicore/drace into feature/dracelinux ([9a7e817b](https://github.com/siemens/drace/commit/9a7e817bb58e641e45736036aab490957f69e370))
*  updates readmes ([aea81c06](https://github.com/siemens/drace/commit/aea81c066264bf98134f85083ad1e1318957a7e8))

##### Performance Improvements

*  use vectors in stackdepot to reduce allocations ([f6450967](https://github.com/siemens/drace/commit/f6450967bcbfa0118c99724ed8c5b764f8e5f4da))

##### Refactors

*  Simplify and cleanup cmake scripts ([35825aa7](https://github.com/siemens/drace/commit/35825aa78a6f3bbd08d79ba58da22122f9444fdd))
*  split shadow stack into logic and instrumentation aspects ([534dbd59](https://github.com/siemens/drace/commit/534dbd591c83762337749137534c29f80cad699d))
*  move enable_external to memory-tracker ([ae247d4e](https://github.com/siemens/drace/commit/ae247d4eafa0c7141502a9d693bc40742331e201))
*  simplified enable/disable logic ([88394664](https://github.com/siemens/drace/commit/88394664f7910c6fe037565affbafdc0f803d617))
*  fixed lots of cppcheck style warnings ([442b5a71](https://github.com/siemens/drace/commit/442b5a71bb4642850324d449fc03e573b63ec254))
*  avoid use of fixed length types ([07189be0](https://github.com/siemens/drace/commit/07189be01f65e34a9b4b217adcf35baa846eb4ff))
*  rework linux changes to be windows compatible ([d0006dcd](https://github.com/siemens/drace/commit/d0006dcde2f1a50619da79b0cc8df75dfa7efdb1))

##### Tests

*  add tests for shadow stack implementation ([1d39c88e](https://github.com/siemens/drace/commit/1d39c88ebf8046642d3663be8a7ace57917b9e3a))
*  use unique names for global shm segments ([15f0bc7c](https://github.com/siemens/drace/commit/15f0bc7c46ed7d8e789843b5bf803700d545ddf9))
*  gather ctest xml results in CI ([e8b81b6c](https://github.com/siemens/drace/commit/e8b81b6c1f793870fb7ebc3e3b9a56d21e503d75))
*  use ctest to execute all unit tests ([18ab8c7f](https://github.com/siemens/drace/commit/18ab8c7fcde63c9e28f5522eff6a6e0878c63a4f))
*  fixed annotation test to incoporate static initialization ([83d14ecf](https://github.com/siemens/drace/commit/83d14ecfc01fca5b43ad21dc3201562072fed3c2))
*  fixed annotation test to incoporate static initialization ([2c9b4a99](https://github.com/siemens/drace/commit/2c9b4a999c98ef1c80e670a02b9e45880e73d649))
*  fix windows specific testing issues ([eadd0a9e](https://github.com/siemens/drace/commit/eadd0a9e20513963771e8953404b07c7a93a2755))
*  prepare integration tests for linux ([1609995e](https://github.com/siemens/drace/commit/1609995ea26ae50cb3198f56b7f265b4516ee860))
*  add support to run detector if tests on linux ([43cf2887](https://github.com/siemens/drace/commit/43cf2887667108f70cc94704d2af3860b2d3da33))

#### 1.5.0 (2020-01-08)

##### Build System / Dependencies

*  excluded some libaries from standalone build ([95d0c548](https://github.com/siemens/drace/commit/95d0c548105c6a40a71cc78381830d2352bd73eb))
*  added a flag to be to build only the stanalone parts ([1e8d97cf](https://github.com/siemens/drace/commit/1e8d97cfc20be2fe8e60f40f8ed36fa0d0c60287))

##### Chores

*  only build necessary deps in standalone version ([efc01ef3](https://github.com/siemens/drace/commit/efc01ef33eb3678e07c8b9a4f4dd97f179cd0170))
*  removed unused variable ([3dde8eb1](https://github.com/siemens/drace/commit/3dde8eb149697a1957a973cab1903a65e17853ee))
*  updated cmake file ([bd857971](https://github.com/siemens/drace/commit/bd857971969e220896522cc41ac9c70e166c8320))
*  adjusted cmake ([5d26d467](https://github.com/siemens/drace/commit/5d26d46743c2b00d6a7ce3d5988cd1cb514f5aa8))
*  introduced typedefs for tid, id, clock ([2c76f6af](https://github.com/siemens/drace/commit/2c76f6af76aa6618b705a3075dec94ed026a3671))
*  resolved compiler warnings ([3b5173ee](https://github.com/siemens/drace/commit/3b5173ee8eb0790ace946d6731351c200c832837))
*  performance improvements of fasttrack ([ec82dcdb](https://github.com/siemens/drace/commit/ec82dcdbf4544d0676440d6ae0de573f31203afd))
*  some style refactorings and added a unit test ([8add9efe](https://github.com/siemens/drace/commit/8add9efe6e8edf6f7b02b90c9026ee66fd9004b8))

##### Documentation Changes

*  updated readme, added limitations ([a5f174cd](https://github.com/siemens/drace/commit/a5f174cdff832f7c2d26971531f30359959c4a9f))
*  improve doxygen html output ([baaabc4a](https://github.com/siemens/drace/commit/baaabc4a6265b40aa8cf2e6196c04979ba060a8f))
*  improved standalone documentation ([17fb2dc0](https://github.com/siemens/drace/commit/17fb2dc0c0ed1ed869a7e8a16f99623e74843adf))
*  updated and added READMEs ([9860c567](https://github.com/siemens/drace/commit/9860c567e8287cd4b0e36be0965c33ddbc54e243))
*  added some documentation ([825d95d8](https://github.com/siemens/drace/commit/825d95d8223decb6672d92289f00667d2f5a40e5))

##### New Features

*  added support for x86 compilation of standalone parts ([4990b68b](https://github.com/siemens/drace/commit/4990b68ba5595a9ea698c94a533f7ad4afc15020))
*  started implementing custom allocators ([36a5d967](https://github.com/siemens/drace/commit/36a5d96751e19fb9552f761bdde9798f6da9a8c0))
*  lock kinds troubleshooting ([32171b52](https://github.com/siemens/drace/commit/32171b52e69a0e2b6683fd3ae35add708ecc2a29))
*  fasttrack tests ([7e8d6744](https://github.com/siemens/drace/commit/7e8d6744f104161618458815bf68f95417ab5600))
*  progress on traacebnary decoder ([ade2d0fe](https://github.com/siemens/drace/commit/ade2d0fed85b7b693893aed6637f7c55889b834b))
*  progress on detector output, but still not entirely working ([7dfe3be6](https://github.com/siemens/drace/commit/7dfe3be6383d520c9298c1b2d9067ca4d1ed7c90))
*  first draft to use the binary decoder with fasttrack ([3912d0e0](https://github.com/siemens/drace/commit/3912d0e05477faa5743520be69ae2114b6f002ad))
*  extended functionality of BinaryDecoder ([8bab5147](https://github.com/siemens/drace/commit/8bab51475ca4c90f0dc60455b096cf66c356ce9d))
*  finalized logging function ([5a380643](https://github.com/siemens/drace/commit/5a380643cf26509557896441da6e5a8d2cbbb585))
*  added logging functionality ([6c3aa01d](https://github.com/siemens/drace/commit/6c3aa01dbfa03ca34ba0a1dd5f16eba36bda55ff))
*  poc for a trace-logging detector ([447ead41](https://github.com/siemens/drace/commit/447ead416eed2448419cb967eaa0990a77f61b43))
*  added cleaning function for stacktrace handling ([cfe41a0f](https://github.com/siemens/drace/commit/cfe41a0f68e262395c02a53247885d69a5ef4640))
*  introduced template for fasttrack to define used lock ([9236fd9a](https://github.com/siemens/drace/commit/9236fd9ab520cee5ca5143427dc5a351eed15782))
*  added reuse of existing nodes in stack tree ([616e0dab](https://github.com/siemens/drace/commit/616e0dabd23223b7b5b1fdb8cf97411e0081017f))
*  stack trace handling was implemented with boost graph library ([10206f06](https://github.com/siemens/drace/commit/10206f06e2238a6cd5b30761b50b2f610161724c))
*  include boost graph ([0cab2b11](https://github.com/siemens/drace/commit/0cab2b11fa0246f83e192f97cb7e3d5f2a2a4827))
*  put tid and clock in one var. to use atomic reads ([e5b9e1c8](https://github.com/siemens/drace/commit/e5b9e1c8781be8292c16bc8dcf93869472b9d286))
*  replaced spinlock with dr_api-locks ([4de4723a](https://github.com/siemens/drace/commit/4de4723af0014c01de41137218e8999d08fbb651))
*  began to tackle requirements of !55 ([2a2717c5](https://github.com/siemens/drace/commit/2a2717c57437635b30698075e6eda9156363c418))
*  further progress on fasttrack ([dfbf4f77](https://github.com/siemens/drace/commit/dfbf4f770252daaf14880d0876f61a7d1463766a))
*  first commit with all potential features of fasttrack ([49fa4a6e](https://github.com/siemens/drace/commit/49fa4a6e8dbfdc3041f6d7782299489004f3172b))
*  further progress on fasttrack ([7101e130](https://github.com/siemens/drace/commit/7101e1300f601082f305b6d672864944826ec0fd))
*  still very early stage of fasttrack algorithm ([cc04e6d8](https://github.com/siemens/drace/commit/cc04e6d86f584b8067d6f4f4845770a1ce1b5c8d))
*  a very early version of the fasttrack algorithm ([f07d8a38](https://github.com/siemens/drace/commit/f07d8a3860e877bc673075767f55c256569ca49f))
*  added plot: errors over time to report ([3a6d88ac](https://github.com/siemens/drace/commit/3a6d88acfdc739f1d75f3110bfe4b96b6c39ff5b))

##### Bug Fixes

*  correctly handle short stack traces in ft ([142953f9](https://github.com/siemens/drace/commit/142953f98aae37380d40bc15584ffb5d752067e8))
*  ensure mutual access per varstate instance ([4c40c5c4](https://github.com/siemens/drace/commit/4c40c5c4d7605b6a31d782516e508c6fae306c80))
*  ft: correctly and efficiently lock data structures ([c0eabaab](https://github.com/siemens/drace/commit/c0eabaab69cdddffb1daed78ef2e163adc0b661a))
*  manually generate export header for detector.h ([a3424a40](https://github.com/siemens/drace/commit/a3424a40e6a531ad7809f9c03871ebb3ce9bc8fb))
*  use static rt in all ft objects ([80205ee3](https://github.com/siemens/drace/commit/80205ee3be2131cfa71b971a575d37e7e37853c9))
*  removed inheritance from shared_mutex ([60b763a9](https://github.com/siemens/drace/commit/60b763a938cef126fe18bd84f86a492f7a54bbf8))
*  resolved cppcheck warnings ([99999f59](https://github.com/siemens/drace/commit/99999f590cad5ab44ead5ddbeb6381c86fa87057))
*  resolved cppcheck warnings ([32e805f8](https://github.com/siemens/drace/commit/32e805f810e72f871061bfaaf4a24963561f0e6c))
*  fixed some issues regarding the binary decoder ([8afe83bb](https://github.com/siemens/drace/commit/8afe83bb412d6f38cc74fe9a429856defb3d35a4))
*  fixed cppcheck issues ([43bb2d81](https://github.com/siemens/drace/commit/43bb2d81f71b78705c8e757868396985211e3bf3))
*  fixed cppcheck issues ([affb8521](https://github.com/siemens/drace/commit/affb8521a041bc4d9ae37d4fe2b0cace4ecdb678))
*  debugging measures ([85e15ed8](https://github.com/siemens/drace/commit/85e15ed8c458b4abb0ae09c9ed2e960519a9a077))
*  example branch to investigate cmake issue ([8e436a50](https://github.com/siemens/drace/commit/8e436a5086aa4beeb695a7b7f3c999ca155d551d))
*  revert erronous change to dllimport ([9e6ce71d](https://github.com/siemens/drace/commit/9e6ce71dfa6c5e33da2c823f656b4677a837854a))
*  properly generate export headers ([2c2c538b](https://github.com/siemens/drace/commit/2c2c538be1cecb701c7509b541f53ddeee316c9b))
*  fixed compile error ([8d940116](https://github.com/siemens/drace/commit/8d940116ee4491c4a2e2ee23e32ee80cda414742))
*  troubleshooting ([02efdb69](https://github.com/siemens/drace/commit/02efdb6982360ba94c2310a86a9c840c40b845de))
*  fixed bug of double insertions of ids into the shared vectorclock of a variable ([f65a851c](https://github.com/siemens/drace/commit/f65a851c2b639e2e0dcf13d1bc4c93ab6747cdfc))
*  fixed cppcheck issues of [#17325502](https://github.com/siemens/drace/pull/17325502) ([64c72893](https://github.com/siemens/drace/commit/64c72893417ade76e02c37ee4fee126b069fab67))
*  set of id in varstate ([b7c458eb](https://github.com/siemens/drace/commit/b7c458eba896d0d3b23836d86fa8a32259da59c9))
*  an existance check of a xml was missing ([4eef966b](https://github.com/siemens/drace/commit/4eef966b0cc5db72a15e16b5bb61f7d9b090ce44))
*  resolved path issue ([6e37c1a0](https://github.com/siemens/drace/commit/6e37c1a02f84f07c65594a974f18d9687f8400ca))

##### Other Changes

*  added file-header to all files ([2bb1caef](https://github.com/siemens/drace/commit/2bb1caef096380321b0c4915be444e5a4fb2fed9))
*  resolved more path issues ([1efe1861](https://github.com/siemens/drace/commit/1efe1861ae698553562fbda990d6116220077fc4))
*  same asone comit earlier ([d56a5459](https://github.com/siemens/drace/commit/d56a5459bdb570dbe7a16844c171b717312cf368))
*  path was not conv. to string for open(...) ([86d70177](https://github.com/siemens/drace/commit/86d70177120db58d709789245bd10064ba90df24))
*  minor bug fix ([80465ee1](https://github.com/siemens/drace/commit/80465ee1b54b3214a0e43a947731f3c055fe27cf))
*  resolve relative path issues ([e1dd0358](https://github.com/siemens/drace/commit/e1dd03584ec25c7f4aa12af542f6978b71c8a087))

##### Performance Improvements

*  improve performance of spinlock ([bcb1b3ac](https://github.com/siemens/drace/commit/bcb1b3ac5d9992585336dd28c988dc0f589c7b11))
*  avoid double-search in stacktrace hot path ([d7965bce](https://github.com/siemens/drace/commit/d7965bceb903366438fe4d860b8d1f691bf0e92a))
*  enable avx2 features on windows ([45ceeac5](https://github.com/siemens/drace/commit/45ceeac5ee3f6d99c6e6afeba93e9ba803de60d3))
*  massively increased performance of ft2 implementation ([07aae5fd](https://github.com/siemens/drace/commit/07aae5fd25d3c845f061c52c00338bd384e7c20a))
*  simplified locking in FT ([81195405](https://github.com/siemens/drace/commit/81195405e0e7f372a2943a2d84eedd4b2ef3edb7))
*  removed race detection for finished thr for performance reasons ([80dbc3ed](https://github.com/siemens/drace/commit/80dbc3edd694caddd529ce2df35e4ebefb94c2ca))
*  several measures to try to increase performance ([44cebf7b](https://github.com/siemens/drace/commit/44cebf7bee3f29c3a2a472056f6cd12e0fd7acec))
*  changed stack trace handling to improve perf. ([3106e098](https://github.com/siemens/drace/commit/3106e098836012e3c63bb219d3cac3274f50be3b))

##### Refactors

*  minor refactorings ([603b683c](https://github.com/siemens/drace/commit/603b683c5e085c7ebd905c0a65c0d9d7c006c929))
*  use brace initialization in varstate ([67a2ba35](https://github.com/siemens/drace/commit/67a2ba35f123e358098efd66dd5ed65204b7ddbd))
*  improve memory usage efficiency ([fd43e6c0](https://github.com/siemens/drace/commit/fd43e6c087fab9c4ebc35e5ed83d500f821e8bb9))
*  ft minor improvements ([2ed6398e](https://github.com/siemens/drace/commit/2ed6398ed0706cf00d50c45160ce00166a23230b))
*  unify name of private variables ([d825e9c1](https://github.com/siemens/drace/commit/d825e9c1a0b317a8d3ba455e0dd6387c90497930))
*  removed  false characters ([2f9b05ca](https://github.com/siemens/drace/commit/2f9b05cab4f6a8f351e9a86d5fcd88e7704e0820))
*  resolved merge conflict ([5d0174a4](https://github.com/siemens/drace/commit/5d0174a4a8981240ab8e3aa367cf8536f9dd5fad))
*  separate build of ft and ft-drace to avoid global-flag changes of DR ([52f02bdc](https://github.com/siemens/drace/commit/52f02bdcba2abfd4529bb8f6a0248dd80a796c4a))
*  give lock type a self explaining name ([87d440b1](https://github.com/siemens/drace/commit/87d440b18cfc1ef55028ba18c168a85fc8696b3c))
*  improved ft install handling ([e8d19ba2](https://github.com/siemens/drace/commit/e8d19ba2bad043a4de0aec854e36c0a7d2503035))
*  improve interface build cmake ([731da4e1](https://github.com/siemens/drace/commit/731da4e1020de9d6f113bda142f3e9eee07803ef))
*  minor refactorings ([d4e81aea](https://github.com/siemens/drace/commit/d4e81aeaf5a3abf31b9a80a2e99e452b1420146b))
*  add parallel hashmap as external dependency ([5e228ffa](https://github.com/siemens/drace/commit/5e228ffa2b851e76531338a0fefa29d345180f4b))
*  minor reconfigurations ([688aced5](https://github.com/siemens/drace/commit/688aced52ce55dca2759615e9f2a2a0967a1e68a))
*  removed fasttrack.cpp as source ([4d42fdfd](https://github.com/siemens/drace/commit/4d42fdfd83451c49482a21ca020a8dfada1218e0))
*  added header and implementation files to all non-template classes ([b744b73a](https://github.com/siemens/drace/commit/b744b73aedc0e8975099a77edcf3196019f30b80))
*  adjusted cmake file of fasttrack ([5c209790](https://github.com/siemens/drace/commit/5c209790bdaee162ed681fceaa566d3247122bed))
*  adjusted fasttrack to new rtload impl. git pull origin feature/reportgui ([28f94d37](https://github.com/siemens/drace/commit/28f94d3745c55982c7a2addf6ba3dcb3b06f4454))
*  minor refactorings ([ea5ab458](https://github.com/siemens/drace/commit/ea5ab45859f88af079317028058c408e3a5aaf34))
*  optimisation with regards to fasttrack, doc and unit test ([ad93aa6d](https://github.com/siemens/drace/commit/ad93aa6d6fe473f25152e34eedabddc624e4ab95))
*  moved placeholders.txt ([1f5d3ed8](https://github.com/siemens/drace/commit/1f5d3ed8a48f29136c9313ef62cbb884b4881dd2))

##### Code Style Changes

*  minor name changes because of python conv. naming ([394038e7](https://github.com/siemens/drace/commit/394038e7351f7aa97729229d727978fa21cfd91b))

##### Tests

*  add support to run detector if tests on unix ([6ce2b3c9](https://github.com/siemens/drace/commit/6ce2b3c9f5eb50caa0d00c98bb6c9efb79ba1104))
*  add support to run detector if tests on unix ([ddab1113](https://github.com/siemens/drace/commit/ddab11133ac47175341b4ba832554ce099062ea8))
*  increased race count of .net/clr_racy test ([9b5179dd](https://github.com/siemens/drace/commit/9b5179dddc525560deb5a7e7fad25a3c2bb589ca))
*  fixed spinlock unit test ([2154012d](https://github.com/siemens/drace/commit/2154012d5fa7d8c6139440adf62b9eef2c54d1ea))
*  added some fasttrack unittests ([a25fc740](https://github.com/siemens/drace/commit/a25fc740234fafbfc04ba412c6c58d94699fc1e7))
*  added stacktrace unit tests ([2f735e76](https://github.com/siemens/drace/commit/2f735e76a687aa7a36ea44132ead72914388125e))
*  fixed behaviour of fasttrack in lock kinds test ([e3b922f8](https://github.com/siemens/drace/commit/e3b922f873a98f901f3fddeee7c3442bc8a3f6ac))
*  added some unit tests ([0da4683e](https://github.com/siemens/drace/commit/0da4683eadb1e5176490c514ba2172e35a1717a7))
*  use static runtime ([070ca89e](https://github.com/siemens/drace/commit/070ca89e3bb76b5328e300e0c0264fb52df12972))
*  bump dr version in CI ([51b7462c](https://github.com/siemens/drace/commit/51b7462c462e114e8e3256059beda8f6dd9f8f1c))
*  encode path to drrun into binary ([5ba76249](https://github.com/siemens/drace/commit/5ba762491b936d2f5f036d419c15fd413e639e7b))
*  add infrastructure for component tests ([cb043c37](https://github.com/siemens/drace/commit/cb043c375308554e63cfc7efac409c0bfae9c7c7))
*  adjusted unit tests to load the standalone version of fasttrack ([c2333ef6](https://github.com/siemens/drace/commit/c2333ef67fe0625798320c6bd325430b9574f102))
*  adjusted unit tests ([f5d4df5c](https://github.com/siemens/drace/commit/f5d4df5cd61bb1efa187ec65fff917a95f57c112))
*  adjusted paths of gui-test ([a009abed](https://github.com/siemens/drace/commit/a009abed9b03c69e54dae61ff9735a19c35dc737))
*  removed python tag from stage ([afba6468](https://github.com/siemens/drace/commit/afba64686ff2c0c68c9fa28f3ce259dd5cf76e42))
*  test-commit for testing of new CI script ([a64dac02](https://github.com/siemens/drace/commit/a64dac0229bd0d68153e3cd05572eeba791926cb))
*  corrected except/only paths ([19a62f3e](https://github.com/siemens/drace/commit/19a62f3e5aaa4b5775461e8a9125606c8db6b4e5))
*  relocated and extended unit_test.py ([606862d5](https://github.com/siemens/drace/commit/606862d582282e1db733d3f7878203ffd44734b4))
*  adjusted syntax error in .gitlab-ci ([5c636036](https://github.com/siemens/drace/commit/5c636036542af9905eca836d19aab888f292cde1))
*  included drace-gui-.gitlab-ci.yml functionality in global .gitlab-ci.yml ([281dab31](https://github.com/siemens/drace/commit/281dab3159dcac042bdf8afdf23ad3e2b46e564d))
*  added first unit test ([34b79fda](https://github.com/siemens/drace/commit/34b79fda2c4023332670905ee42d837a90a4a00e))

#### 1.4.0 (2019-12-13)

##### Chores

*  add support for vs enterprise ([4c9be969](https://github.com/siemens/drace/commit/4c9be969a99bc51b45859983cd535ecbad7003dc))
*  adjusted naming of tests ([6d2a30ff](https://github.com/siemens/drace/commit/6d2a30ff95425e705a141383781c0511de025250))
*  use vs enterprise in ci ([54f878a3](https://github.com/siemens/drace/commit/54f878a386d4412b67dd9f8c57f0a5d1a024565f))
*  removed qt path from cmake file, updated readme ([97bf1348](https://github.com/siemens/drace/commit/97bf13488d88d59a822f454d885e53aa1abc9cdc))
*  only build gui if boost is found ([0bc6d076](https://github.com/siemens/drace/commit/0bc6d076d5695d72afcca8486fed1d4f74bf876f))
*  set correct error code in CI ([98c390d7](https://github.com/siemens/drace/commit/98c390d7647677bf3314ca0186d3f2c1fdad9c6a))
*  add support to deploy self-containing version of drace-gui ([7e9bb50c](https://github.com/siemens/drace/commit/7e9bb50cfc9f313c4b6af5dc61715e8499949fce))
*  integrate draceGUI into cmake project ([3b267d9a](https://github.com/siemens/drace/commit/3b267d9aa62bcfd4ce67abad3dbd591ff21f4f0b))
*  generate binary of reportgenerator ([ed9dd604](https://github.com/siemens/drace/commit/ed9dd6043fad35f2c112d6b37e16fbf2342c5539))

##### New Features

*  auto select drace dll if next to gui ([dd476c0e](https://github.com/siemens/drace/commit/dd476c0eeb09724a06a7b748fe27aa51940b3b58))
*  added 'race' suppression classifier ([35ed138e](https://github.com/siemens/drace/commit/35ed138e8ac2d026abe0e823099ea7f4946932d4))
*  added wild card and special character handling ([dca4d598](https://github.com/siemens/drace/commit/dca4d598ed9540272e67cbf4290bb118ceb093fe))
*  added working dummy test, set global_var ([36aa205f](https://github.com/siemens/drace/commit/36aa205ffd66bae2dcb3e5a8fec3f425af75d967))
*  implemented race filter ([c29e8b6c](https://github.com/siemens/drace/commit/c29e8b6c8239982de6c300f57ff4a3edd3a75e23))
*  added load/save functionality ([26568f24](https://github.com/siemens/drace/commit/26568f24a5126b33a26ac49815edc20590c6bff8))
*  added report creation, added msr functionality ([37212333](https://github.com/siemens/drace/commit/372123333d1aa7bf62d7c284ccc9bdd40629c11f))
*  further progress on gui features ([b590576f](https://github.com/siemens/drace/commit/b590576f956e44ed22f9ae726f2265d8dd3825ba))
*  add boost dependency in drace-gui ([2e44c490](https://github.com/siemens/drace/commit/2e44c490d873524239a5697f949f2e103a4d229a))
*  changed architecture, reorganized folder ([9334ec49](https://github.com/siemens/drace/commit/9334ec4920af964005f95659dda5f18baf00d1d2))
*  added action buttons with dummy windows ([206c689f](https://github.com/siemens/drace/commit/206c689fb7aefb72409c8d15f994aee5605a2d20))
*  further progress on drace gui ([23b5830c](https://github.com/siemens/drace/commit/23b5830c7ee4c6f2f5a2f57ee1150070fdc5b9e7))
*  additional files for drace gui ([9ff810aa](https://github.com/siemens/drace/commit/9ff810aacb1f220a44272f6a9d5f338f8300318e))
*  added draceGUI source files ([ea82cc19](https://github.com/siemens/drace/commit/ea82cc1956111a37e377dda689e739b6c9aaee17))
*  tool was renamed to ReportConverter, some usability improvements ([d61b08f1](https://github.com/siemens/drace/commit/d61b08f10fdc03d33d00d6e1b8d74e7eec8989c3))
*  fallback to config file at drace binary location ([d85c6bea](https://github.com/siemens/drace/commit/d85c6bea603986004cabcf7e9afb9af4acf1564e))

##### Bug Fixes

*  replace angular ticks in html output ([d8bbbaa5](https://github.com/siemens/drace/commit/d8bbbaa52591aa9d09cf0425192cde0e2f751228))
*  escape all html-incompatible characters ([5ffb67bc](https://github.com/siemens/drace/commit/5ffb67bcc1deb5894aad50b9fd80b634b312089d))
*  removed inheritance from shared_mutex ([8c53af39](https://github.com/siemens/drace/commit/8c53af3968b1b8719cf1adf84faf53cb2edd4a11))
*  removes vertical lines, when no matplotlib pictures are created ([aecd1601](https://github.com/siemens/drace/commit/aecd1601ec975264509f62bcf67b3019ab1ae2bf))
*  fixes double quote issue after load ([784ea468](https://github.com/siemens/drace/commit/784ea4689403c028d7f0644c85efe2c9075d0767))
*  properly parse race-free report ([e1bc335a](https://github.com/siemens/drace/commit/e1bc335af4334b21dee5701c4184bf7ba29db35d))
*  solved issue with paths containing spaces ([6b967523](https://github.com/siemens/drace/commit/6b967523e74b6afb9b2908e23a4a56864f2a3ae7))
*  resolved findings of cpp-check of Pipeline [#3859543](https://github.com/siemens/drace/pull/3859543) ([51538fe8](https://github.com/siemens/drace/commit/51538fe8fc7a8094e0b2bf94d1a58374822e8020))
*  msr handling was fixed ([1a875b47](https://github.com/siemens/drace/commit/1a875b4781b8345086be67277ab15ee3ed491e49))
*  the created .exe finds now the resources path ([eb1628e6](https://github.com/siemens/drace/commit/eb1628e6814b048556c154f89f9e3b96119744df))
*  make codebase cppcheck compliant ([5010bd80](https://github.com/siemens/drace/commit/5010bd80b431b6a7bb9b68d41d321c08e2bcb06c))
*  removed circular include ([c4563523](https://github.com/siemens/drace/commit/c4563523316cc0394e9b87a676c48fe97540a012))
*  check if license path is set was added ([ea8bf43b](https://github.com/siemens/drace/commit/ea8bf43be3891e51728788b62fd8b4fafd003e97))
*  correctly exit on errors in pyinstaller builds ([b24f7ce1](https://github.com/siemens/drace/commit/b24f7ce10e98753f8458118dfa1d97fdcfc1f563))
*  output path in debug mode was corrected ([0332835d](https://github.com/siemens/drace/commit/0332835d3c553ad8ae1d351a11e1118a34e4aaab))

##### Other Changes

*  updated readme.md ([8fa5ad17](https://github.com/siemens/drace/commit/8fa5ad1734a4583c899e3007441d11cf370b5a7f))
*  adjusted Readme-files, due to renaming of report generator ([07077745](https://github.com/siemens/drace/commit/07077745423998469da23f7b7f31b0efb6a6d15e))

##### Refactors

*  fix shadow of variable ([39ba21b5](https://github.com/siemens/drace/commit/39ba21b55be42924af42ac63ecb4a567b2ff1a42))
*  prepare symbol subsystem to be mocked ([58b674a4](https://github.com/siemens/drace/commit/58b674a45248aab950daff60780c3a94136722cc))
*  split symbol module into separate components ([c8ecb533](https://github.com/siemens/drace/commit/c8ecb533a7bf46a132bbb57d290d39ffbf749d55))
*  used scoped dynamorio lock in race-collector ([b084c426](https://github.com/siemens/drace/commit/b084c426d242a2876529710217eee1274e9275f3))
*  minor adjustments, style and doc ([022d8f19](https://github.com/siemens/drace/commit/022d8f194b5572cc12eac54c8ee1ccbb1ff95ba3))
*  adapt unit tests to new testing infrastructure ([3011d233](https://github.com/siemens/drace/commit/3011d233279dc2a544acfa10703bedfcb92b7517))
*  finished renaming ([ca288337](https://github.com/siemens/drace/commit/ca28833763e4156c49deb3297b5bfb98d745f746))

##### Code Style Changes

*  resolved cppcheck warning ([1a01abd7](https://github.com/siemens/drace/commit/1a01abd74fad39fe2089a615c646f8335d9751ed))
*  minor style fixes ([52cd5f6a](https://github.com/siemens/drace/commit/52cd5f6a77d9cb75d48b20996bdb2b019bd5ff3e))

##### Tests

*  improve robustness of integration test infrastructure ([79821737](https://github.com/siemens/drace/commit/79821737eddd836062fc772ffd4c58cd84020bc3))
*  setup ctest for drace-unit-tests ([a8fbe0d7](https://github.com/siemens/drace/commit/a8fbe0d7084d6906c92fb309d86e73c95381b43a))
*  test race-supression component ([7ac416f6](https://github.com/siemens/drace/commit/7ac416f60240efe8892fb6ba91c5b98d53fca5b3))
*  use boost process to control MSR ([06689d61](https://github.com/siemens/drace/commit/06689d61414f84ff871db2e151284b5fc434a27f))
*  retry system tests in case of sporadic crash ([4fe1cfe5](https://github.com/siemens/drace/commit/4fe1cfe5a089411655ea5543ffa4c7fcb96e1ce4))
*  split unit and integration tests ([76c41e4c](https://github.com/siemens/drace/commit/76c41e4c46a281d558dadcd13c03d27073cd87ce))
*  encode path to drace into test binary ([557892ec](https://github.com/siemens/drace/commit/557892ec2b7b94fb98d0f2cfbd98bc88c9d524e2))
*  use static runtime ([98f7533e](https://github.com/siemens/drace/commit/98f7533e14fee2f7fef6341bcdd294c96886aeb0))
*  bump dr version in CI ([9ad7a47e](https://github.com/siemens/drace/commit/9ad7a47efd75787f70ec16c0182db79b1e4a4a08))
*  encode path to drrun into binary ([bcb85f12](https://github.com/siemens/drace/commit/bcb85f1228008269cc2cd2a071fe6bf1935c13c8))
*  encode path to drrun into binary ([ea31ff45](https://github.com/siemens/drace/commit/ea31ff452cd53dd77a0105702eda40060f462ae6))
*  added boost path to cmake generation ([b1b45003](https://github.com/siemens/drace/commit/b1b4500366859bb8f8aeb6c115bd7e87fb71fd6a))
*  adjusted Unittests and Cmake ([5d239c41](https://github.com/siemens/drace/commit/5d239c41518d9e837a072fd89ca9ac9d50d0b278))

#### 1.2.0 (2019-07-11)

##### Chores

*  fixed CI build pathes ([fee0bb96](https://github.com/siemens/drace/commit/fee0bb96a3c140a00de9099c188cab6e0900e1dd))
*  bump drace version to 1.2.0 ([dda19fae](https://github.com/siemens/drace/commit/dda19faedb2ac3aa3d6ac280d404bfb5e01cf610))
*  bump dynamorio version to 7.91.x ([87ad21a8](https://github.com/siemens/drace/commit/87ad21a8701ce4400a6d69d3fde51238314cd701))
*  prepare drace-gui for integration into drace repo ([96d04beb](https://github.com/siemens/drace/commit/96d04beb096a04eef375c76c702ed6930a5eb4c2))
*  add cmake option to install tests and benchs ([2d61f530](https://github.com/siemens/drace/commit/2d61f530f6cf38de6128d1319fb842a7e681d58b))
*  bump dynamorio version in CI ([1581d15b](https://github.com/siemens/drace/commit/1581d15b0b5a6ac6aa41bfa4a81ca42c219dc5a1))
*  adapt benchmark to recent detector if changes ([31262410](https://github.com/siemens/drace/commit/312624101f1d31a4c4a6a99fe2fe267dd0f296e3))
*  add package step in ci ([f2a85e78](https://github.com/siemens/drace/commit/f2a85e78c70d2fade3e8328be90126638efa6a00))

##### Documentation Changes

*  added missing submodule init command to documentation ([ef1ea842](https://github.com/siemens/drace/commit/ef1ea842b89cf68c93e0c30d0fb2e94e06a4406d))
*  added entry for drace-gui in global drace readme.md ([c2db8619](https://github.com/siemens/drace/commit/c2db861959fceec282149d052e872123e6ae50e6))
*  Created Repository ([b9b113f4](https://github.com/siemens/drace/commit/b9b113f457b53007d2d599a5640862972445e7df))
*  Created Repository ([3257a4e3](https://github.com/siemens/drace/commit/3257a4e37507319c7ed7f74c4113d60246180459))
*  added note on expected curiosities on coreclr app ([547419f0](https://github.com/siemens/drace/commit/547419f0c7addc7178d7cc9086fc476f6adefc17))

##### New Features

*  added timestamp to xml report ([cac0a642](https://github.com/siemens/drace/commit/cac0a642bdb4903c6e2685e837995c4f284a6cd4))
*  added highlighting of active stack element ([46b438ed](https://github.com/siemens/drace/commit/46b438ed7278807aa1e8ff34fb5f8724b29f43f3))
*  added line offset to xml report ([44cef244](https://github.com/siemens/drace/commit/44cef244264e1fe022e52ad28c99533b4d383648))
*  added button to switch between stack entries of one error entry ([fc52ecf4](https://github.com/siemens/drace/commit/fc52ecf4c23b54c9149481f288cc56202cc31e35))
*  added vscode:// and file:// protocoll, added license header ([d41bb86d](https://github.com/siemens/drace/commit/d41bb86d6c515cb1415fe8d7868cf7fe1463ecfb))
*  cs-sync project file ([9f86d31c](https://github.com/siemens/drace/commit/9f86d31c5cb63f095512932d155c10aa070772b0))
*  added link to open file with vs code ([dd94443b](https://github.com/siemens/drace/commit/dd94443bf7e884d24defa2d82a0809c575f1cafc))
*  added legend picture to output ([1a5648de](https://github.com/siemens/drace/commit/1a5648de731c570ebb380ca87288c09df00b43eb))
*  Added bar chart of occurencences of top of stack functions ([c8c4a65e](https://github.com/siemens/drace/commit/c8c4a65e37c118a8c22fde76f86dda32fd034c02))
*  Added BL/WL input flags ([dd81ff9d](https://github.com/siemens/drace/commit/dd81ff9d348cb18c541b3d84b96f308c6ce9beeb))
*  added code snipping ([5f21fe15](https://github.com/siemens/drace/commit/5f21fe15d1ff7620f9ab1d6aa0dc34563ffc437a))
*  Changed DEBUG mode behaviour ([0bc99dd3](https://github.com/siemens/drace/commit/0bc99dd317fa80d18876674d666673d34f72617a))
*  Added i/o args ([9526ff9a](https://github.com/siemens/drace/commit/9526ff9a1bbbe1d2423f4dbb917f8b5973479b73))
*  Initial commit of css files needed for report creation ([495a6d3d](https://github.com/siemens/drace/commit/495a6d3d796bcf124ea507eb1fce65069384b229))
*  Initial commit of js files needed for report creation ([161939e7](https://github.com/siemens/drace/commit/161939e743750f99629c70bc93524c89c12da12d))
*  added new placeholders (for overview reasons) ([ab034b6b](https://github.com/siemens/drace/commit/ab034b6b37caf1b8674f4b36dc15ab623697d58d))
*  edited according to drace-gui.py ([e9287fb8](https://github.com/siemens/drace/commit/e9287fb8e44781fe7b9a5ec3af0d5e5081a33003))
*  added black/whitelisting, labeling of error boxes, SC box @ same height, file @ correct line, line highlighting, grey shading ([983ad0d9](https://github.com/siemens/drace/commit/983ad0d94b2f7b4f279c3f2329cc04566dfa266c))
*  initial commit drace-gui.py, example.html, entries.xml ([63665639](https://github.com/siemens/drace/commit/63665639cbc539e7370b51ebb8e74d55ea10e366))
*  track thread creation using HB ([8a8cb081](https://github.com/siemens/drace/commit/8a8cb08132fcfbd984949b09bf62c9301e6a580a))
*  added single threaded dummy application ([2a038ca3](https://github.com/siemens/drace/commit/2a038ca35e82e7be7d93246e565d79f36415c184))
*  removed legacy detection mode ([d318a623](https://github.com/siemens/drace/commit/d318a62338459279c37752fa0ba35777a6809c6d))
*  add option to display per-thread statistics ([6686d943](https://github.com/siemens/drace/commit/6686d9436356f2b0fba2c4be8028f742500f6192))
*  implement streaming support for race reports ([7853f3de](https://github.com/siemens/drace/commit/7853f3dec8f25696cb3c1ef0fa6b694e61bc1159))

##### Bug Fixes

*  correctly free mutex hashtable (memory-leak) ([80d4a3c7](https://github.com/siemens/drace/commit/80d4a3c711c82a1c06f3b27b4006cfb2f6b30476))
*  deallocate buffer with correct size ([4453a942](https://github.com/siemens/drace/commit/4453a942ceec48be7a06bcebe85d146a6298068a))
*  creation of chart is just executed when matplotlib is installed ([c27205a8](https://github.com/siemens/drace/commit/c27205a8cd4aff50673b9ba1b29430f1a4d299d6))
*  corrected exception type ([e683f7ff](https://github.com/siemens/drace/commit/e683f7ff886c1d012e823095fbb27b1355be3aec))
*  fixed ci-file ([b5a6123b](https://github.com/siemens/drace/commit/b5a6123b22ad6a4e9c80667ef04b56bf4ac5f4ee))
*  Corrected intentional bug to test CI ([de4d345d](https://github.com/siemens/drace/commit/de4d345d30ae8eb991898fb480ddea944852198b))
*  Adjusted line-height parameter of code preview ([07641f76](https://github.com/siemens/drace/commit/07641f76c8b76af52ea814c0f05007a02c5a9323))
*  Adjusted number for scroll offset calculaton ([e2dc3e55](https://github.com/siemens/drace/commit/e2dc3e55aaf3a738666c17cad4483f294fca081c))
*  exclude dotnet system.linq.expressions as call instrumentation often crashes ([8aa183e9](https://github.com/siemens/drace/commit/8aa183e9899d10ceb6eb25f8f8888a5a88b269a7))
*  use dynamorio to allocate string buffers ([e81f079d](https://github.com/siemens/drace/commit/e81f079d263ce366db9243e168930e6714037608))
*  carved out some memory misusage ([f78f9bf1](https://github.com/siemens/drace/commit/f78f9bf1eebec103792137f5a4292cb15e1c9bb2))
*  do not determine stack range for first / initial thread ([c6403e41](https://github.com/siemens/drace/commit/c6403e41363fcb4682276574486910fc50e30a00))
*  corrected format specifiers in xml printer ([074ea20a](https://github.com/siemens/drace/commit/074ea20a4ddbe5d2399e07e203e618904cd482b9))
*  corrected possible memory error in log printing ([709c9f10](https://github.com/siemens/drace/commit/709c9f10fe939cf766712ac49c331ed6fbb4cac5))
*  replace stringstream in valkyries sink ([60eba4ec](https://github.com/siemens/drace/commit/60eba4ecd351e5725dbfe9d75458c62b39de8d3f))
*  use c-style formatting to work around i[#9](https://github.com/siemens/drace/pull/9) ([57ee864c](https://github.com/siemens/drace/commit/57ee864cff773ddccd9c64de60daec51a582c041))
*  do not use stringstream in hr-sink ([da7558de](https://github.com/siemens/drace/commit/da7558deacc4269ce6f3b1399943dc41cca2f6dc))
*  avoid double locking in tracker ([3c0a99c4](https://github.com/siemens/drace/commit/3c0a99c4c2db1020f7f129b9b59e43b2ec0fa51b))
*  only print stats if explicitly enabled ([d096a880](https://github.com/siemens/drace/commit/d096a880f9784fb6537cda0a446b8de12364c8ea))
*  data-races and double locking ([e4e6b018](https://github.com/siemens/drace/commit/e4e6b0187b009c4851b1dfb91e268485740549e2))
*  make race-collector thread-safe ([8e72ff23](https://github.com/siemens/drace/commit/8e72ff23571d8edbd6a9f10e23e8dd5eda258105))
*  correctly report thread id in tsan detector ([f20cd093](https://github.com/siemens/drace/commit/f20cd093349850436f37b30c3277d97a5022e4ec))
*  properly handle exceptions in wrapped functions ([89a8fc42](https://github.com/siemens/drace/commit/89a8fc424fe6faae8002e9ea79365390137ed8b3))
*  correctly print messages in non mingw terminals ([bf5cce0a](https://github.com/siemens/drace/commit/bf5cce0a9eb144758ede67d7b4c4d1163f728104))

##### Other Changes

*  corrected input path  debug mode ([272456d0](https://github.com/siemens/drace/commit/272456d0a865254beef83cff735093265f9a442e))
*  corrected syntax error ([9c6b23b3](https://github.com/siemens/drace/commit/9c6b23b30747356e4e536dff255b8fe6358d273b))
*  check of existance of vscode in try/except ([ebc11269](https://github.com/siemens/drace/commit/ebc11269c61f049f5af4f27e279128178a541209))
*  added vs code info, updated black/whitelisting entry ([5ae386e1](https://github.com/siemens/drace/commit/5ae386e176f4f2775cfc7fa949ea00aa20774152))

##### Performance Improvements

*  remove heap only option to speed up allocations and deallocations ([9ff015b5](https://github.com/siemens/drace/commit/9ff015b5c49b072389618de72724261130a29414))
*  reduce memory usage of race collector in streaming mode ([a958c75e](https://github.com/siemens/drace/commit/a958c75e94fa5008f45d37f7f9a4ecc697e943bb))
*  optimize cache locality of inline instrumentation ([ae5290b6](https://github.com/siemens/drace/commit/ae5290b6684b85b9fe339816b1c3c95891a6a0cf))
*  add switch to avoid pdb symbol lookup in many modules ([8658a745](https://github.com/siemens/drace/commit/8658a74599223987637ab36af609674550ebb407))

##### Refactors

*  removed any 'os' module dependencies, improved datestring generation ([b617040e](https://github.com/siemens/drace/commit/b617040ed2a185b1987fcfce9bd4408bcc6e12e3))
*  output of report in single folder to not throw several files in the output folder ([835adb53](https://github.com/siemens/drace/commit/835adb53b4a668581619ecd0dc58533e74dadc44))
*  removed old example file ([eb8e9a66](https://github.com/siemens/drace/commit/eb8e9a663590b1161117a7612bc32df0c6f5da4f))
*  changed style to siemens look ([db38b37f](https://github.com/siemens/drace/commit/db38b37fe65d19f27bf78a84069b2538d13804ec))
*  black and whitelisting procedure was refactored ([5e4e6aca](https://github.com/siemens/drace/commit/5e4e6aca8a46bc3ff9044f37b5258c29ba5b5c4f))
*  changed appearance of directory string ([0943e653](https://github.com/siemens/drace/commit/0943e65302f6d63ccec939dc817d44ec24bc4f57))
*  adjusted font size and button margins ([6ce33c94](https://github.com/siemens/drace/commit/6ce33c94f04e668f70c7e8babe21e43352a72170))
*  line height for scroll offset is obtained by js ([9b5afaab](https://github.com/siemens/drace/commit/9b5afaaba56b59ba7adac99b6d65adc5f84bf934))
*  adjusted button height ([55fe8ef2](https://github.com/siemens/drace/commit/55fe8ef249e4b3ee310e768ad67be832455af963))
*  adjusted bar chart display functionality ([4844245c](https://github.com/siemens/drace/commit/4844245cf7f41031b1605de93e5bb325cb70e657))
*  minor refactoring ([467d246b](https://github.com/siemens/drace/commit/467d246b4fa6889c57d85b0ace3e909fcf878efa))
*  added feature and design changes ([1c5d58db](https://github.com/siemens/drace/commit/1c5d58dba6c48b9a7b9f5a1c532502e8e6e2ff64))
*  updated placeholders ([a739418b](https://github.com/siemens/drace/commit/a739418bab8ac5ac2807ae05a2dc2e310d49878f))
*  tsan: map addresses into go-heap range ([48466f80](https://github.com/siemens/drace/commit/48466f801466c92748a3d550d7fcff62593c62e3))
*  remove threadid tracking in statistics ([5a18ca66](https://github.com/siemens/drace/commit/5a18ca66034596302c99c76a14e482c02dfc567f))
*  fix const-correctness of module cache ([29998d55](https://github.com/siemens/drace/commit/29998d5595e921b4525e965dc26be026c13f3262))
*  avoid non-dr allocations if possible ([eeb141ff](https://github.com/siemens/drace/commit/eeb141ff5073ebd938dc34f9f3c561f3abbab926))
*  temporary disable tid tracking to avoid allocations ([d414192c](https://github.com/siemens/drace/commit/d414192c145f09c6da65b82998bf843da0bf5bf8))
*  lock access to drsym functions ([71ea3100](https://github.com/siemens/drace/commit/71ea310055aba83d96d439771894e6b433c4862b))
*  allocate symbol info using dr ([19ebf0ed](https://github.com/siemens/drace/commit/19ebf0eda6dd4c37fcf8736d913886ffec0d6cba))
*  improve compatibility of memory instrumentation with DR7.90.18033 ([5ec5f942](https://github.com/siemens/drace/commit/5ec5f9421f55b5a5af72cd85f9c0ebea4b2b239e))
*  temporary disable allocation tracking in TSAN detector ([d6c2f80b](https://github.com/siemens/drace/commit/d6c2f80b9ae8e5e638522ac05dc89e1bdf1af2c4))
*  allocate buffer on the heap ([2a516141](https://github.com/siemens/drace/commit/2a516141084f0a8e13a142b3e4ea82fd65ed7491))
*  safe fp-state in clean call ([d1afc7c1](https://github.com/siemens/drace/commit/d1afc7c1679e6b208e9f3b7f5175f2343a93dfa8))
*  replaced all stringstream with c-style printing in valkyries sink ([8671c4e4](https://github.com/siemens/drace/commit/8671c4e4bc495aeec79f9c508cdcbe020c032332))
*  remove external locking on allocation/dealloc in detector ([9494800c](https://github.com/siemens/drace/commit/9494800c59dbe80fb53f618dddbccd25ba261ed4))
*  removed lots of dead code ([1317c24c](https://github.com/siemens/drace/commit/1317c24c8115ca1fd2eb8b08daefb5bbae325ea7))
*  change memory allocation in race collector ([3969c27e](https://github.com/siemens/drace/commit/3969c27e19362f92e19c85f48225293e75d168dd))
*  cleanup unused variables and dead code ([92da494f](https://github.com/siemens/drace/commit/92da494fb6e3b21fc493ac2dba807cef8cd42e78))
*  use correct types in shadow stack callbacks ([55b24ab7](https://github.com/siemens/drace/commit/55b24ab7104497760156c9ee4672013a8bb67fe3))
*  add wrapper for dynamorio file handles ([da6d1def](https://github.com/siemens/drace/commit/da6d1def414b0513c690b2c8114c512fb160c1a8))
*  re-design data-race reporting pipeline ([fe7e0127](https://github.com/siemens/drace/commit/fe7e01270cf77d8a8ae96b7ec69e2f0806d87d0b))

##### Tests

*  added Debug flag to call in pipeline file ([ee44a3d1](https://github.com/siemens/drace/commit/ee44a3d195e7789ab6b8449d3b11b3f672502d77))
*  corrected gitlab-ci.yml syntax ([270b795c](https://github.com/siemens/drace/commit/270b795cbe6ef3ea836657a0c525a588989be531))
*  Added gitlab-ci.yml ([adea41f4](https://github.com/siemens/drace/commit/adea41f4e8f7a41a528010e73daf90fd01dca017))
*  smaller test file (only 4 entries) ([5ca62871](https://github.com/siemens/drace/commit/5ca62871cb08b527733cf9ee6c326da7ed035ce4))
*  fixed variable shadow in memchecker ([f3ecdc24](https://github.com/siemens/drace/commit/f3ecdc248c8df71e62aa13fe5e53ebc23e6de10d))
*  optimized memory checker app ([94df8c71](https://github.com/siemens/drace/commit/94df8c719b2c560f73bb8daaefe57abd7adc2825))
*  added memory checker testing application ([e5a6d865](https://github.com/siemens/drace/commit/e5a6d8654f2d1e5bc78edd275102a3b3d50c38ad))
*  bump dr to 7.90 in CI build ([6a10cc61](https://github.com/siemens/drace/commit/6a10cc614c019eb66d393034ca82ea25c1268a52))
*  disable dotnet tests in legacy build ([23ac4d00](https://github.com/siemens/drace/commit/23ac4d001f1c7da024e8f951c5895fbd6a548df8))
*  delayed symbol lookup ([c62e33b8](https://github.com/siemens/drace/commit/c62e33b83f5c70daf102eb70241e6dc274f312f6))
*  check reported state in xml output ([df880608](https://github.com/siemens/drace/commit/df880608b809c8e79119bb56dbf13ce1a21a8cf9))
*  re-enable tid check in xml report ([3a2148fb](https://github.com/siemens/drace/commit/3a2148fb6d268b66a7744c8fa8194d2078d66a7c))
*  fix type in report xml integration test ([a0cecf17](https://github.com/siemens/drace/commit/a0cecf176d805d4214bc2a2d5f46788609f569fd))
*  verify race report (text) ([7a3e2998](https://github.com/siemens/drace/commit/7a3e2998b1e65b939a0948ee4c144179ffd7d18b))
*  document cli of sampler application ([e65f227e](https://github.com/siemens/drace/commit/e65f227e2d655232f031f03356d3ce1ed0ab2eda))
*  check equality of threadids of tsan and drace ([3b687071](https://github.com/siemens/drace/commit/3b6870715931fe62d7ac53f04efc150fa6c9c992))

#### 1.1.1 (2019-04-05)

##### Chores

*  install target in CI ([f5431bc1](https://github.com/siemens/drace/commit/f5431bc1ac78a884c3d66d2d20dde1128deaa86c))
*  build using vs professional in CI ([40c01cb1](https://github.com/siemens/drace/commit/40c01cb1c9d360ee0cda24801f8d411612d94cb4))
*  install runtime (dll) only ([3c014434](https://github.com/siemens/drace/commit/3c014434e92307f0b8598a23ec5700f7055015d2))

##### Documentation Changes

*  add note on symbol search path [ci skip] ([87c1f54c](https://github.com/siemens/drace/commit/87c1f54c6edb605d0c9b8476e067b5c9a7cc372d))

##### New Features

*  issue warning if sym search path does not contain sym server ([acf9a507](https://github.com/siemens/drace/commit/acf9a507dd820d4166b3a7ae7e36a1efc080206e))
*  wait some time for msr to become ready ([3e865546](https://github.com/siemens/drace/commit/3e865546af0512a941977ce213492998c9f12a57))
*  add flag to run msr just once ([9a0b63cd](https://github.com/siemens/drace/commit/9a0b63cd417606285250fe932da8bbbffcf32f54))
*  reduce redundancy in ci scripts ([5b0cd7d3](https://github.com/siemens/drace/commit/5b0cd7d33ac11634b04f756cdf5035240811ca45))
*  improve data-race report format ([b2bf9957](https://github.com/siemens/drace/commit/b2bf99573e08929e93dbce9b6fd57693e6ecde18))
*  unify size of shadow stack ([306a8ac7](https://github.com/siemens/drace/commit/306a8ac7cb52daa22c905bab58475d89a8a39ecb))
*  issue a warning if no dotnet sync functions are wrapped ([36438d32](https://github.com/siemens/drace/commit/36438d3209443162bba8a2f110c777d9780a113b))
*  improve loading of dbghelp.dll to get ms symbol server support ([085a8af7](https://github.com/siemens/drace/commit/085a8af770a0ba02b3a16cd68fb9ee0d682a7b79))
*  run managed application even if msr is not running ([583060f6](https://github.com/siemens/drace/commit/583060f65cdad7f932343efd8c27e66fbc65c22f))
*  add limited support for windows 7 ([0ed77b97](https://github.com/siemens/drace/commit/0ed77b9747eda22eb9eb591ccccf10a7cfb4266d))

##### Bug Fixes

*  correctly use SymSearch function ([63ee8547](https://github.com/siemens/drace/commit/63ee854771583613f75cd7e73c33a67cdceb8e4c))
*  do not print statistics on thread exit [#2](https://github.com/siemens/drace/pull/2) ([e9fb9532](https://github.com/siemens/drace/commit/e9fb953223c8dec4d65676b0b17b7d47e4934cec))

##### Code Style Changes

*  re-format document ([7028aba3](https://github.com/siemens/drace/commit/7028aba35ae04a1775021320f20ac099f4241856))

##### Tests

*  skip dotnet tests in dr7.0 CI runs as not supported by dr7.0.x ([1f2b4ad5](https://github.com/siemens/drace/commit/1f2b4ad50895bff6e88ddae3759c92fb5296f770))
*  set sym search path in CI ([4e850c67](https://github.com/siemens/drace/commit/4e850c67dc766ed2badf91b6775b64c627aea56d))
*  print drrun output on failure in verbose mode ([b47b9aa3](https://github.com/siemens/drace/commit/b47b9aa3c0cbf1a60cd10f4729b717868ecc7598))
*  bump dr version to support clr ([c9a2ce93](https://github.com/siemens/drace/commit/c9a2ce93df0bb31fb7bdd115010b0169b080f38c))
*  add CLR tests in CI suite ([cbba289c](https://github.com/siemens/drace/commit/cbba289c75c357f701bfd89abbce50e2657b126d))
*  improve dotnet testing and infrastructure ([8fe44a41](https://github.com/siemens/drace/commit/8fe44a418a78b4529098345b8a83cc1b93d4d433))
*  run some tests only on master to improve CI time ([cbc0d2b6](https://github.com/siemens/drace/commit/cbc0d2b643f0126e2acfb293d3139c841654edcb))
*  improve ci time ([504b4032](https://github.com/siemens/drace/commit/504b4032976fc5fe8956b4fe4173d8ab81b99045))
*  split testing into separate steps ([a63a2733](https://github.com/siemens/drace/commit/a63a273341151c0c235e4b8379aede40c49a7344))

#### 1.1.0 (2019-03-25)

##### Chores

*  bump tinyxml version, directly use provided lic file ([6b3d7f82](https://github.com/siemens/drace/commit/6b3d7f82160c5c0e34abcc95e99024415fa8b5cc))
*  build dummy detector in CI ([0f119b37](https://github.com/siemens/drace/commit/0f119b37fb578188792e23bb1a21087617393349))
*  always check if copy of dlls is necessary ([a7704736](https://github.com/siemens/drace/commit/a77047366e99f47ec9459487eba10281409654ea))
*  describe static analysis and add CII badge ([557a3c09](https://github.com/siemens/drace/commit/557a3c0953e0c06fcc25731153c6c95d1f446e77))
*  enforce git check if version file not found ([1cc65848](https://github.com/siemens/drace/commit/1cc65848d676ec558946519d1dd120a5ad392343))
*  improved git burn-in script ([2e47ef5e](https://github.com/siemens/drace/commit/2e47ef5e4d5a147ec7109d07a95ef4967cb21e1e))

##### Documentation Changes

*  further clarify dependencies when using pre-build releases ([09c2c742](https://github.com/siemens/drace/commit/09c2c742859a9e753cb4a61e8b07fea268186de3))
*  update changelog ([baf32c46](https://github.com/siemens/drace/commit/baf32c46e13c78096ffb19f03ecde915da9cf507))
*  clarify supported dynamorio versions ([adbcc27f](https://github.com/siemens/drace/commit/adbcc27fbfe91986a558471994485c9088c86f35))
*  update github issue url ([438ea99b](https://github.com/siemens/drace/commit/438ea99b223f5ce15f1444ab715f50b72a1e62a3))
*  note on semantic versioning ([1780a398](https://github.com/siemens/drace/commit/1780a398e7f4987941c40efa8c6a33d73bdf6c5a))
*  add SPDX license file ([f97ba5dc](https://github.com/siemens/drace/commit/f97ba5dce9d2e0442d6a2b700208a7f6395ef730))
*  added reference to conv. changelog ([1d8c2269](https://github.com/siemens/drace/commit/1d8c2269d919a483a0b302c98c6bb586d62a54ff))
*  make licensing information SPDX compatible ([c078bb16](https://github.com/siemens/drace/commit/c078bb16415008053785608d082f414185cd5854))
*  updated licensing information ([d03267e4](https://github.com/siemens/drace/commit/d03267e4a271c864be4e869d8fa063c8ab41b2b4))
*  add list of maintainers ([61417cca](https://github.com/siemens/drace/commit/61417cca8eecc525fec49db91d6f325ae8f597b6))
*  how to cite drace ([5b1ab201](https://github.com/siemens/drace/commit/5b1ab201ea36dba6b58b88bf1a64b7e92c171368))

##### New Features

*  move handle specific logic to tsan ([d1f7abe4](https://github.com/siemens/drace/commit/d1f7abe457ff30891eee7b57bb0b15e8aec05d53))
*  cleanup and streamline tsan-drace modifications ([c12fc0e7](https://github.com/siemens/drace/commit/c12fc0e7341cd5ae15555029c870de670fd51d46))
*  add release and debug version of tsan ([2d1cbb5b](https://github.com/siemens/drace/commit/2d1cbb5b198ccff8058fd3be2f3e5b296bcc888b))
*  add option to control race suppressions ([0b6a3e17](https://github.com/siemens/drace/commit/0b6a3e178e4b578e8b4940398e0dfb1c9c8a8d94))

##### Bug Fixes

*  adapt dummy detector to recent if changes ([523e6cae](https://github.com/siemens/drace/commit/523e6cae512751f34c2bc4ef180fb8bacd3fd80b))
*  resolve many shadow mapping issues ([b993025a](https://github.com/siemens/drace/commit/b993025a9ae6f62d9b19e62bf9a55cdcae292fab))
*  resolve memory leak in managed resolver ([9abf7336](https://github.com/siemens/drace/commit/9abf73363269fb5ae7ffc3dd5023af26af86c8af))
*  resolve linking issue in versioning script ([e75bc001](https://github.com/siemens/drace/commit/e75bc0015a0f3915352133a175e8171fba75e748))
*  do not log after logfile is closed ([53a6afa3](https://github.com/siemens/drace/commit/53a6afa34f5a7954bf1f2e151aa69f69786f3a9f))

##### Other Changes

*  fix error in congested spinlock ([3811650c](https://github.com/siemens/drace/commit/3811650cea6d2a4f403dae04444e94d2a3b0026f))

##### Refactors

*  bump version number due to change in detector if ([e7694757](https://github.com/siemens/drace/commit/e76947575b7ba28916f302dab15feef2cce518fa))
*  fix issues reported by cppcheck ([a6e2f4a0](https://github.com/siemens/drace/commit/a6e2f4a07d8bc7454d0a7aa1f1c0406406fa9e09))
*  fix compiler warnings ([c6f3464d](https://github.com/siemens/drace/commit/c6f3464d88db47f01f4975719cfa3196979736e4))
*  fix compiler warnings in msr ([ba482a98](https://github.com/siemens/drace/commit/ba482a98f16ba09f5aab84ef06ef507929431bb7))

##### Code Style Changes

*  re-format code ([e42de5d0](https://github.com/siemens/drace/commit/e42de5d0fe2c5e22fd5b0de5b89d3190f9691c55))
*  re-format file ([57b5d639](https://github.com/siemens/drace/commit/57b5d639d04dab470d6ec9cf4ad5d0f7fcfe2f6b))
*  properly format file ([0647ebf4](https://github.com/siemens/drace/commit/0647ebf426a02748479357ca8b89b57bbc0db0f7))
*  cleanup formatting in drace-tsan ([c019c48a](https://github.com/siemens/drace/commit/c019c48ae2927b67db5b5bd6124273b1069a3b9c))

##### Tests

*  test client against dr 7.0 and 7.1 ([80a9b20b](https://github.com/siemens/drace/commit/80a9b20b59b20661fef3ee17539f3e2e3f6b46a3))
*  added in-depth test of race report ([1e5ecb68](https://github.com/siemens/drace/commit/1e5ecb68fcf0d94da201b06d38659aac2310d371))
*  disable racyAtomics test due to many spurious failures ([39d6ba89](https://github.com/siemens/drace/commit/39d6ba8923483992a9c802238015b05ad2abcf05))
*  re-enabled racy-atomics test ([15aa5522](https://github.com/siemens/drace/commit/15aa5522253a9704c120a73f913845fa5d581a81))
*  improved test by enforcing data-race ([a195f6bf](https://github.com/siemens/drace/commit/a195f6bfff924f05bea25c2a72e2ce2339edd0ef))
*  re-enabled racy-atomics test ([1d0d7cd4](https://github.com/siemens/drace/commit/1d0d7cd4388ab05506b4c496a79c9c9888f85707))
*  output if data-race acutally happened, enforce race ([d1d0ef86](https://github.com/siemens/drace/commit/d1d0ef867f1b45d2c12beb12d4f2307ef0e8d03c))
*  ensure copy of altered detector dlls in build step ([e66d6756](https://github.com/siemens/drace/commit/e66d675629d2110c3d051401c673af7b1d3a8736))
*  access only heap addresses ([a0c4a624](https://github.com/siemens/drace/commit/a0c4a624ff3f9dd5659dcab59715d8d74596f426))
*  fix annotation integration test ([98880589](https://github.com/siemens/drace/commit/98880589b71378e5ba5e9aa1dd072cce11d45413))
*  print cppcheck results on cl ([a11e01e8](https://github.com/siemens/drace/commit/a11e01e884a12bfb7eb03c92da444c52592c817a))
*  split CI ([acd3f784](https://github.com/siemens/drace/commit/acd3f78403e6f5f7210efe14dcab18a9a68e0f80))
*  enable cppcheck in CI ([bb74ddaa](https://github.com/siemens/drace/commit/bb74ddaabfc76d32609c306a57762750b62107df))
*  copy suppressions file to binary dir ([db8ff7e5](https://github.com/siemens/drace/commit/db8ff7e591ff9fcfce2d74472eeab4b91a3d650b))
*  add cppcheck support in cmake project ([a6d408f3](https://github.com/siemens/drace/commit/a6d408f31b2856f5c87168eeba330eb46f8cc469))
*  fix gtest xml ci ([f490f6a9](https://github.com/siemens/drace/commit/f490f6a9f2b76783f1763239e61bfb4aed538abd))
*  upload gtest results as artifact ([788c0627](https://github.com/siemens/drace/commit/788c0627bb01af88078db70c300c29c34b811010))
*  setup local CI ([5f559487](https://github.com/siemens/drace/commit/5f559487abe8121169a07967bb65b9516a1cc726))

#### 1.0.3

* Initial OSS release (see README.md for details)

