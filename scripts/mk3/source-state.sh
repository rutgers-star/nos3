# Record the source revision at build time so package.sh cannot label old binaries as the current checkout.
mk3_source_state() (
    cd "$mk3_repo"
    export LC_ALL=C
    mk3_commit=$(git rev-parse HEAD)
    mk3_diff_hash=$(git diff HEAD --binary --ignore-submodules=none | sha256sum | awk '{print $1}')
    printf 'repository_commit=%s\nrepository_tracked_diff_sha256=%s\n' "$mk3_commit" "$mk3_diff_hash"
    echo submodules_begin
    git submodule status --recursive
    echo submodules_end
)
