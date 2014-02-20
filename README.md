A simpler case for an ingest might be to take an RSS feed and create objects for each entry.

### Basic process for implementing batches

* Identify structure for source to ingest.
* Figure out what of the structure will be broken out into individual objects
* Write preprocessor and object classes, such that:
    * The preprocessor breaks up the objects into in parts which the "object"
      class can work on
    * The object class takes what the preprocessor gives it and performs
      minimal processing during instantiation, and has a `batchProcess()`
      method which does what little heavy lifting there might be (excluding
      derivatives, which should be generated via
      `hook_islandora_derivative()`).

### Usage of this module

The code in this module can actually run, though it is a bit messy. The
preprocessor can be run in the Devel PHP, with the snippet:

```php
$preprocessor = new RSSBatchPreprocessor(islandora_get_tuque_connection(), array());
$preprocessor->preprocess();
```

To actually run the batch ingest, on the command line, run:

`drush islandora_batch_ingest`
