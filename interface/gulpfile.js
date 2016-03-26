var gulp = require('gulp'),
  connect = require('gulp-connect'),
  del = require('del'),
  gzip = require('gulp-gzip'),
  inject = require('gulp-inject'),
  mainBowerFiles = require('main-bower-files');

gulp.task('copy', function() {

  gulp.src('./app/*.js')
    .pipe(gulp.dest('./build'));

});

gulp.task('inject', ['copy'], function() {

  var bower_components = gulp.src(
    mainBowerFiles()
  );

  gulp.src('./app/index.html')
    .pipe(
      inject(
        bower_components
        .pipe(gulp.dest('./build/')), {
          name: 'bower',
          ignorePath: 'build/',
          addRootSlash: false
        })
    )
    .pipe(gulp.dest('./build'));

});

gulp.task('package', ['inject'], function() {

  gulp.src('./build/**/*')
    .pipe(gzip())
    .pipe(gulp.dest('./package'));

});

gulp.task('webserver', ['inject'], function() {

  connect.server({
    root: './build'
  });

  gulp.watch('app/**', ['inject']);

});

gulp.task('clean', function() {

  del(['build', 'package']);

});

gulp.task('unclutter', ['clean'], function() {

  del(['bower_components', 'node_modules']);

});

gulp.task('default', ['webserver']);
