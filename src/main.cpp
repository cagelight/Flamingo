#include "qa_core.hpp"

int main(int argc, char *argv[]) {
    FlamingoCore app(argc, argv);
    app.setOrganizationName("Sensory Systems");
    app.setOrganizationDomain("snsys.us");
    app.setApplicationName("Flamingo");
    app.setApplicationVersion("Pre-Alpha");
    return app.exec();
}
