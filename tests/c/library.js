mergeInto(LibraryManager.library, {
  allocFunction: function(size) {},
  source: function() { return 42},
  sink: function(y) {},
  predicate: function() {return 42},
});